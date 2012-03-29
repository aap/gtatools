Camera = {}
mt = {}

function Camera:new()
	return setmetatable({}, mt)
end

function Camera:panLR(d)
	__cameraPanLR(d)
end

function Camera:panUD(d)
	__cameraPanUD(d)
end

function Camera:turnLR(phi)
	__cameraTurnLR(phi)
end

function Camera:turnUD(phi)
	__cameraTurnUD(phi)
end

function Camera:moveInOut(d)
	__cameraMoveInOut(d)
end

function Camera:setPitch(pitch)
	__cameraSetPitch(pitch)
end

function Camera:setYaw(yaw)
	__cameraSetYaw(yaw)
end

function Camera:setDistance(d)
	__cameraSetDistance(d)
end

function Camera:setTarget(d)
	__cameraSetTarget(d)
end

function Camera:setFov(fov)
	__cameraSetFov(fov)
end

function Camera:getPitch()
	return __cameraGetPitch()
end

function Camera:getYaw()
	return __cameraGetYaw()
end

function Camera:getDistance()
	return __cameraGetDistance()
end

function Camera:getTarget()
	return __cameraGetTarget()
end

function Camera:getPosition()
	return __cameraGetPosition()
end

mt.__index = Camera

cam = Camera:new()
