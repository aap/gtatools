--
-- Camera
--
cam = {}
function cam.panLR(d) __cameraPanLR(d) end
function cam.panUD(d) __cameraPanUD(d) end
function cam.turnLR(phi) __cameraTurnLR(phi) end
function cam.turnUD(phi) __cameraTurnUD(phi) end
function cam.moveInOut(d) __cameraMoveInOut(d) end
function cam.setPitch(pitch) __cameraSetPitch(pitch) end
function cam.setYaw(yaw) __cameraSetYaw(yaw) end
function cam.setDistance(d) __cameraSetDistance(d) end
function cam.setTarget(x, y, z) __cameraSetTarget(x, y, z) end
function cam.setFov(fov) __cameraSetFov(fov) end
function cam.getPitch() return __cameraGetPitch() end
function cam.getYaw() return __cameraGetYaw() end
function cam.getDistance() return __cameraGetDistance() end
function cam.getTarget() return __cameraGetTarget() end
function cam.getFov(fov) return __cameraGetFov() end
function cam.getPosition() return __cameraGetPosition() end

--
-- World
--
world = {}
function world.setInterior(i) __worldSetInterior(i) end
function world.getInterior() return __worldGetInterior() end

--
-- Gl
--
gl = {}
function gl.setDoTextures(i) __glSetDoTextures(i) end
function gl.getDoTextures() return __glGetDoTextures() end
function gl.setDoZones(i) __glSetDoZones(i) end
function gl.getDoZones() return __glGetDoZones() end
function gl.setDoFog(i) __glSetDoFog(i) end
function gl.getDoFog() return __glGetDoFog() end
function gl.setDoVertexColors(i) __glSetDoVertexColors(i) end
function gl.getDoVertexColors() return __glGetDoVertexColors() end
function gl.setDoCol(i) __glSetDoCol(i) end
function gl.getDoCol() return __glGetDoCol() end


dofile("init.lua")
