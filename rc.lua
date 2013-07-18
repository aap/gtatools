GTA3 = 1
GTAVC = 2
GTASA = 3

--
-- Camera
--
cam = {}
-- function cam.panLR(d) __cameraPanLR(d) end
-- function cam.panUD(d) __cameraPanUD(d) end
-- function cam.turnLR(phi) __cameraTurnLR(phi) end
-- function cam.turnUD(phi) __cameraTurnUD(phi) end
-- function cam.moveInOut(d) __cameraMoveInOut(d) end
-- function cam.setPitch(pitch) __cameraSetPitch(pitch) end
-- function cam.setYaw(yaw) __cameraSetYaw(yaw) end
-- function cam.setDistance(d) __cameraSetDistance(d) end
-- function cam.setTarget(x, y, z) __cameraSetTarget(x, y, z) end
-- function cam.setFov(fov) __cameraSetFov(fov) end
-- function cam.getPitch() return __cameraGetPitch() end
-- function cam.getYaw() return __cameraGetYaw() end
-- function cam.getDistance() return __cameraGetDistance() end
-- function cam.getTarget() return __cameraGetTarget() end
-- function cam.getFov(fov) return __cameraGetFov() end
-- function cam.getPosition() return __cameraGetPosition() end
-- function cam.Print() return __cameraPrint() end
-- function cam.lock(i) return __cameraLock(i) end
function cam.setPosition() return __cameraSetPosition(x, y, z) end
function cam.getPosition() return __cameraGetPosition() end
function cam.setTarget(x, y, z) __cameraSetTarget(x, y, z) end
function cam.getTarget() return __cameraGetTarget() end
function cam.turn(a, b) return __cameraTurn(a, b) end
function cam.orbit(a, b) return __cameraOrbit(a, b) end
function cam.dolly(a) return __cameraDolly(a) end
function cam.zoom(a) return __cameraZoom(a) end
function cam.setFov(fov) __cameraSetFov(fov) end
function cam.getFov(fov) return __cameraGetFov() end
function cam.lock(i) return __cameraLock(i) end

--
-- World
--
world = {}
function world.setInterior(i) __worldSetInterior(i) end
function world.getInterior() return __worldGetInterior() end

--
-- Time
--
time = {}
function time.setHour(i) __timeSetHour(i) end
function time.getHour(i) return __timeGetHour() end
function time.setMinute(i) __timeSetMinute(i) end
function time.getMinute(i) return __timeGetMinute() end
function time.setCurWeather(i) __timeSetCurrentWeather(i) end
function time.getCurWeather(i) return __timeGetCurrentWeather() end

--
-- Renderer
--
renderer = {}
function renderer.setDoTextures(i) __rendererSetDoTextures(i) end
function renderer.getDoTextures() return __rendererGetDoTextures() end
function renderer.setDoZones(i) __rendererSetDoZones(i) end
function renderer.getDoZones() return __rendererGetDoZones() end
function renderer.setDoFog(i) __rendererSetDoFog(i) end
function renderer.getDoFog() return __rendererGetDoFog() end
function renderer.setDoVertexColors(i) __rendererSetDoVertexColors(i) end
function renderer.getDoVertexColors() return __rendererGetDoVertexColors() end
function renderer.setDoCol(i) __rendererSetDoCol(i) end
function renderer.getDoCol() return __rendererGetDoCol() end
function renderer.setDoTrails(i) __rendererSetDoTrails(i) end
function renderer.getDoTrails() return __rendererGetDoTrails() end
function renderer.setDoBFC(i) __rendererSetDoBFC(i) end
function renderer.getDoBFC() return __rendererGetDoBFC() end
function renderer.setLodMult(f) __rendererSetLodMult(f) end
function renderer.getLodMult() return __rendererGetLodMult() end
