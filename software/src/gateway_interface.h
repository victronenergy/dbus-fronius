#ifndef GATEWAY_INTERFACE_H
#define GATEWAY_INTERFACE_H

/*!
 * An interface for all classes supporting network wide device detection.
 *
 * This class has been created to decouple several classes supporting device detection.
 * It should not be confused with the `AbstractDetector` class, which supports device detection
 * on a single host. Classes implementing `GatewayInterface` scan multiple hosts, using
 * `AbstractDetector` classes to scan a single host.
 */
class GatewayInterface {
public:
	virtual ~GatewayInterface();

	virtual void startDetection() = 0;
};

#endif // GATEWAY_INTERFACE_H
