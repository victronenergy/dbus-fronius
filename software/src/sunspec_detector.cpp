#include <velib/vecan/products.h>
#include "modbus_tcp_client.h"
#include "modbus_reply.h"
#include "sunspec_updater.h"
#include "sunspec_detector.h"
#include "sunspec_tools.h"

SunspecDetector::SunspecDetector(QObject *parent):
	AbstractDetector(parent),
	mUnitId(0)
{
}

SunspecDetector::SunspecDetector(quint8 unitId, QObject *parent):
	AbstractDetector(parent),
	mUnitId(unitId)
{
}

DetectorReply *SunspecDetector::start(const QString &hostName, int timeout)
{
	return start(hostName, timeout, mUnitId);
}

DetectorReply *SunspecDetector::start(const QString &hostName, int timeout, quint8 unitId)
{
	Q_ASSERT(unitId != 0);

	// If we already have a connection to this inverter, then there is
	// no need to scan it again.
	if (SunspecUpdater::hasConnectionTo(hostName, unitId)) {
		return 0;
	}

	ModbusTcpClient *client = new ModbusTcpClient(this);
	connect(client, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(client, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	client->setTimeout(timeout);
	client->connectToServer(hostName);
	Reply *reply = new Reply(this);
	reply->client = client;
	reply->di.networkId = unitId;
	reply->di.hostName = hostName;
	mClientToReply[client] = reply;
	return reply;
}

void SunspecDetector::onConnected()
{
	ModbusTcpClient *client = static_cast<ModbusTcpClient *>(sender());
	Reply *di = mClientToReply.value(client);
	Q_ASSERT(di != 0);
	di->state = Reply::SunSpecHeader;
	di->currentRegister = 40000;
	startNextRequest(di, 2);
}

void SunspecDetector::onDisconnected()
{
	ModbusTcpClient *client = static_cast<ModbusTcpClient *>(sender());
	Reply *di = mClientToReply.value(client);
	if (di != 0)
		setDone(di);
}

void SunspecDetector::onFinished()
{
	ModbusReply *reply = static_cast<ModbusReply *>(sender());
	Reply *di = mModbusReplyToReply.take(reply);
	reply->deleteLater();

	QVector<quint16> values = reply->registers();

	switch (di->state) {
	case Reply::SunSpecHeader:
	{
		QString sunspecId;
		if (values.size() == 2)
			sunspecId = getString(values, 0, 2);
		if (sunspecId != "SunS") {
			setDone(di);
			return;
		}
		di->currentRegister += 2;
		di->state = Reply::ModuleContent;
		startNextRequest(di, 66);
		break;
	}
	case Reply::ModuleHeader:
	{
		if (values.size() < 2) {
			// If we get a short frame, it means there was an error reading
			// something. As long as we have enough to keep going, call
			// setResult anyway. This helps SMA inverters which errors
			// when reading one register past the end, instead of returning
			// 0xFFFF as most other implementations do.
			if ( !di->di.productName.isEmpty() && // Model 1 is present
					di->di.phaseCount > 0 && // Model 1xx present
					di->di.networkId > 0)
				di->setResult();
			setDone(di);
			return;
		}
		quint16 modelId = values[0];
		quint16 modelSize = values[1];
		di->state = Reply::ModuleHeader;
		switch (modelId) {
		case 101:
		case 102:
		case 103:
		case 111:
		case 112:
		case 113:
			di->di.retrievalMode = modelId > 103 ? ProtocolSunSpecFloat : ProtocolSunSpecIntSf;
			di->di.phaseCount = modelId % 10;
			di->di.inverterModelOffset = di->currentRegister;
			break;
		case 701:
			// If we already detected a 100-model, stick with that.
			if (di->di.phaseCount > 0)
				break;
			di->di.retrievalMode = ProtocolSunSpec2018;
			di->di.inverterModelOffset = di->currentRegister;
			// Call startNextRequest directly and ask for only 3 registers.
			// This avoids the issue that model 701 is 153 long, too long for
			// a single request.
			startNextRequest(di, 3); // We Only need ACType
			di->state = Reply::ModuleContent;
			return;
		case 120: // Nameplate ratings
		case 702: // IEEE 1547 DERCapacity page
			// If we already know the max power, no need to fetch it again.
			if (di->di.maxPower > 0)
				break;
			di->state = Reply::ModuleContent;
			break;
		case 123: // Immediate controls
			// SMA is always breaking model 123. Let's completely ignore
			// model 123 to prevent issues with unreadable registers.
			// Since model 1 always comes first, the productId will
			// already be populated.
			if (di->di.productId != VE_PROD_ID_PV_INVERTER_SMA) {
				di->di.immediateControlOffset = di->currentRegister;
				di->state = Reply::ModuleContent;
			}
			break;
		case 0xFFFF:
			if ( !di->di.productName.isEmpty() && // Model 1 is present
					di->di.phaseCount > 0 && // Model 1xx present
					di->di.networkId > 0)
				di->setResult();
			setDone(di);
			return;
		}
		if (di->state == Reply::ModuleHeader) {
			di->currentRegister += 2 + modelSize;
			startNextRequest(di, 2);
		} else {
			startNextRequest(di, modelSize + 2);
		}
		break;
	}
	case Reply::ModuleContent:
		if (values.size() < 1) {
			setDone(di);
			return;
		}
		di->state = Reply::ModuleHeader;
		quint16 modelId = values[0];
		switch(modelId) {
		case 1:
			if (values.size() == 66) {
				QString manufacturer = getString(values, 2, 16);
				if (manufacturer == "Fronius")
					di->di.productId = VE_PROD_ID_PV_INVERTER_FRONIUS;
				else if (manufacturer == "SMA")
					di->di.productId = VE_PROD_ID_PV_INVERTER_SMA;
				else if ((manufacturer == "ABB") || (manufacturer == "FIMER"))
					di->di.productId = VE_PROD_ID_PV_INVERTER_ABB;
				else
					di->di.productId = VE_PROD_ID_PV_INVERTER_SUNSPEC;
				QString model = getString(values, 18, 16);
				di->di.productName = QString("%1 %2").arg(manufacturer).arg(model);

				// Fronius uses 'options' (offset 34) for the data manager version
				if (di->di.productId == VE_PROD_ID_PV_INVERTER_FRONIUS) {
					di->di.dataManagerVersion = getString(values, 34, 8);
				}

				di->di.firmwareVersion = getString(values, 42, 8);
				di->di.uniqueId = di->di.serialNumber = getString(values, 50, 16);
			}
			break;
		case 701: // DERMeasureAC
			if (values.size() > 2)
				di->di.phaseCount = values[2] + 1;
			break;
		case 120: // Nameplate ratings
			if (values.size() > 4)
				di->di.maxPower = getScaledValue(values, 3, 1, 4, false);
			if (values.size() > 22)
				di->di.storageCapacity = getScaledValue(values, 21, 1, 22, false);
			break;
		case 702: // DERCapacity, new IEEE 1547 alternative for 120
			if (values.size() > 45)
				di->di.maxPower = getScaledValue(values, 2, 1, 45, false);
			break;
		case 123: // Immediate controls
			if (values.size() > 23)
				di->di.powerLimitScale = 100.0 / getScale(values, 23);
			break;
		}
		di->currentRegister += 2 + values[1];
		startNextRequest(di, 2);
		break;
	}
}

void SunspecDetector::startNextRequest(Reply *di, quint16 regCount)
{
	ModbusReply *reply = di->client->readHoldingRegisters(di->di.networkId, di->currentRegister,
														  regCount);
	mModbusReplyToReply[reply] = di;
	connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
}

void SunspecDetector::setDone(Reply *di)
{
	if (!mClientToReply.contains(di->client))
		return;
	di->setFinished();
	disconnect(di->client);
	mClientToReply.remove(di->client);
	di->client->deleteLater();
}

SunspecDetector::Reply::Reply(QObject *parent):
	DetectorReply(parent),
	client(0),
	state(SunSpecHeader),
	currentRegister(0)
{
}

SunspecDetector::Reply::~Reply()
{
}
