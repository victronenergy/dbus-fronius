#include "products.h"
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
	di->currentRegister = di->nextSunspecStartRegister();
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
		if (values.size() != 2 || getString(values, 0, 2) != "SunS") {
			// Check the next expected location for a sunspec header
			quint16 next = di->nextSunspecStartRegister();
			if (next == 0xFFFF) {
				setDone(di);
				return;
			}
			di->currentRegister = next;
			startNextRequest(di, 2);
			return;
		}
		di->currentRegister += 2;
		requestNextHeader(di); // Probably model 1
		break;
	}
	case Reply::ModuleHeader:
	{
		if (values.size() < 2) {
			// If we get a short frame, it means there was an error reading
			// something. Check if we have enough to still detect an inverter.
			// This helps SMA inverters that errors when reading two registers
			// at the end, instead of returning a zero-length end model
			// (0xFFFF, 0) as per the spec.
			checkDone(di);
			return;
		}
		quint16 modelId = values[0];
		quint16 modelSize = values[1] + 2;
		quint16 nextModel = di->currentRegister + modelSize;
		switch (modelId) {
		case 1:
			// SolarEdge breaks the spec by having multiple device definitions
			// without an end-model marker in between. If we see model 1 come
			// by a second time, that means we're seeing the next device. Try
			// to finish the detection. If we already have model 1 and a
			// measurement model this will succeed.
			if (di->di.productId != 0) {
				checkDone(di);
				return;
			}
			requestNextContent(di, 1, nextModel, modelSize); // Common model
			return;
		case 101:
		case 102:
		case 103:
		case 111:
		case 112:
		case 113:
			di->di.retrievalMode = modelId > 103 ? ProtocolSunSpecFloat : ProtocolSunSpecIntSf;
			di->di.phaseCount = modelId % 10;
			di->di.inverterModel = modelId;
			di->di.inverterModelOffset = di->currentRegister;
			break;
		case 701:
			// If we already detected a 100-model, stick with that.
			if (di->di.phaseCount > 0)
				break;
			di->di.retrievalMode = ProtocolSunSpec2018;
			di->di.inverterModel = modelId;
			di->di.inverterModelOffset = di->currentRegister;
			// Ask for only 3 registers.  This avoids the issue that model 701
			// is 153 long, too long for a single request.
			requestNextContent(di, 701, nextModel, 3); // We Only need ACType
			return;
		case 120: // Nameplate ratings
		case 702: // IEEE 1547 DERCapacity page
			// If we already know the max power, no need to fetch it again.
			if (di->di.maxPower > 0)
				break;
			requestNextContent(di, modelId, nextModel, modelSize);
			return;
		case 704: // DERCtlAC
			if (di->di.immediateControlModel > 0)
				break;
			di->di.immediateControlOffset = di->currentRegister;
			di->di.immediateControlModel = modelId;
			requestNextContent(di, modelId, nextModel, 1, 54); // 1 register at offset 54
			return;
		case 123: // Immediate controls, preferred over 704 if we have both
			// Fetch just one record from offset 23. This avoids all the
			// pain we get from SMA, where some registers in model 123
			// are not readable.
			di->di.immediateControlOffset = di->currentRegister;
			di->di.immediateControlModel = modelId;
			requestNextContent(di, modelId, nextModel, 1, 23);
			return;
		case 0xFFFF:
			checkDone(di);
			return;
		}

		// Next header
		di->currentRegister += modelSize;
		requestNextHeader(di);
		break;
	}
	case Reply::ModuleContent:
		if (values.size() < 1) {
			// SMA often have all sorts of unreadable stuff in model 123, so
			// if there is an error, but we still have at least common info
			// and a measurement model, we call checkDone instead of setDone
			// to at least check if we can continue.
			checkDone(di);
			return;
		}
		switch(di->currentModel) {
		case 1:
			if (values.size() >= 66) {
				QString manufacturer = getString(values, 2, 16);
				if (manufacturer == "Fronius")
					di->di.productId = VE_PROD_ID_PV_INVERTER_FRONIUS;
				else if (manufacturer == "SMA")
					di->di.productId = VE_PROD_ID_PV_INVERTER_SMA;
				else if ((manufacturer == "ABB") || (manufacturer == "FIMER"))
					di->di.productId = VE_PROD_ID_PV_INVERTER_ABB;
				else if (manufacturer.startsWith("SolarEdge"))
					di->di.productId = VE_PROD_ID_PV_INVERTER_SOLAREDGE;
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
			if (values.size() > 0)
				di->di.powerLimitScale = 100.0 / getScale(values, 0);
			break;
		case 704: // DERCtlAC
			if (values.size() > 0)
				di->di.powerLimitScale = 100.0 / getScale(values, 0);
			break;
		}
		di->currentRegister = di->nextModelRegister;
		requestNextHeader(di);
		break;
	}
}

void SunspecDetector::requestNextHeader(Reply *di)
{
	di->state = Reply::ModuleHeader;
	di->currentModel = 0;
	di->nextModelRegister = 0;
	startNextRequest(di, 2);
}

void SunspecDetector::requestNextContent(Reply *di, quint16 currentModel, quint16 nextModelRegister, quint16 regCount, quint16 offset)
{
	di->state = Reply::ModuleContent;
	di->currentModel = currentModel; // model being fetched
	di->nextModelRegister = nextModelRegister;
	di->currentRegister += offset;
	startNextRequest(di, regCount);
}

void SunspecDetector::startNextRequest(Reply *di, quint16 regCount)
{
	ModbusReply *reply = di->client->readHoldingRegisters(di->di.networkId, di->currentRegister,
														  regCount);
	mModbusReplyToReply[reply] = di;
	connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
}

void SunspecDetector::checkDone(Reply *di)
{
	if ( !di->di.productName.isEmpty() && // Model 1 is present
			di->di.phaseCount > 0 && // Model 1xx present
			di->di.networkId > 0)
		di->setResult();
	setDone(di);
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
	currentRegister(0),
	currentModel(0),
	nextModelRegister(0),
	startRegisters(QList<quint16>() << 40000 << 50000 << 0)
{
}

void SunspecDetector::Reply::setFinished()
{
	startRegisters.clear();
	emit finished();
}

quint16 SunspecDetector::Reply::nextSunspecStartRegister()
{
	if (!startRegisters.isEmpty())
		return startRegisters.takeFirst();
	return 0xFFFF;
}

SunspecDetector::Reply::~Reply()
{
}
