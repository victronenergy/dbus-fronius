#ifndef GATEWAY_INTERFACE_H
#define GATEWAY_INTERFACE_H

class GatewayInterface {
public:
	virtual ~GatewayInterface();

	virtual void startDetection() = 0;
};

#endif // GATEWAY_INTERFACE_H
