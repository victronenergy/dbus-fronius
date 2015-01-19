#ifndef FRONIUSDEVICEINFO_H
#define FRONIUSDEVICEINFO_H

struct FroniusDeviceInfo
{
	int deviceType;
	bool supports3Phases;
	const char *name;

	static const FroniusDeviceInfo *find(int deviceType);
};

#endif // FRONIUSDEVICEINFO_H
