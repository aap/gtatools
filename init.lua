--renderer.setDoCol(1)

--cam.setTarget(0, 0, 0)
--cam.setFov(50)
--cam.setDistance(10.0)
--cam.setPitch(3.14/16.0-3.14/2.0)
--cam.setYaw(3)

--cam.setTarget(-67.353, -1546.43, 10.4012)

--cam.setTarget(0,0,0)
--cam.setDistance(5)

--cam.setTarget(978.271, -793.424, 34.443)
--cam.setDistance(30)

--cam.setTarget(1.3148150444031, -1211.2624511719, 16.229518890381)
--cam.setDistance(30)
--cam.setYaw(2.885)
--cam.setPitch(4.7874)

-- ocean drive street
--cam.setTarget(201.83615112305, -1307.6668701172, 10.273765563965)
--cam.setDistance(7.26)
--cam.setYaw(1.5758)
--cam.setPitch(4.875)

-- la records
--cam.setTarget(1176.17, -1154.5, 87.2194)

--casinoblock41
--cam.setTarget(2143.6335449219, 2144.6479492188, 28.93208694458)

-- blueberry
--cam.setTarget(335.5654907, -159.0345306, 17.85120964) 

--cam.setTarget(0,0,0)
--cam.setTarget(-10000, -10000, -10000)

-- cull test gta3
-- cam.setTarget(1093.2956542969,-666.26354980469,51.432041168213)
-- cam.setDistance(0.0)
-- cam.setPitch(4.9388446807861)
-- cam.setYaw(5.4316854476929)

-- cull test gtasa
-- cam.setTarget(-2294.6906738281,807.28759765625,97.711242675781)
-- cam.setDistance(0.0)
-- cam.setPitch(5.4859175682068)
-- cam.setYaw(4.9777021408081)

-- alpha z-fight
-- cam.setTarget(1119.72,-949.591,30.0514)
-- cam.setDistance(10)
-- cam.setYaw(4.71871)
-- cam.setPitch(5.35087)

-- grove street, home
-- cam.setTarget(2479.76,-1682.85,20.0498)
-- cam.setDistance(25)
-- cam.setYaw(5.43921)
-- cam.setPitch(4.9744)

--time.setHour(21);

-- grove 2
-- cam.setTarget(2470.51,-1661.09,23.8617)
-- cam.setDistance(25)
-- cam.setYaw(5.00443)
-- cam.setPitch(4.83322)

-- stahl house
-- cam.setTarget(1343.89,-652.72,112.653)
-- cam.setDistance(25)
-- cam.setYaw(0.00662338)
-- cam.setPitch(4.71579)

-- anim ped
-- cam.setTarget(-138.687,-176.729,16.3685)
-- cam.setDistance(10)
-- cam.setYaw(0)
-- cam.setPitch(4.91531)

--cam.setTarget(2490.86,-1671.93,12.2308)
--cam.setDistance(10)
--cam.setYaw(5.49126)
--cam.setPitch(5.26236)

-- physics
--cam.setTarget(0,0,55);
--cam.setDistance(50)
--cam.setYaw(0)
--cam.setPitch(4.91531)

-- casino royale
--cam.setTarget(2039.92,1434.37,56.9988)
--cam.setDistance(0)
--cam.setYaw(5.1724)
--cam.setPitch(5.05643)

if game == GTASA then
-- start
	cam.setPosition(2235.79,-1262.167,24.7386)
	cam.setTarget(2248.79,-1261.29,24.1036)
	cam.setFov(55)
	time.setCurWeather(13)
	time.setHour(6)
	time.setMinute(52)
end

if game == GTAVC then
	cam.setFov(55)
	time.setCurWeather(4)
	time.setHour(7)
	time.setMinute(18)

-- physics
--	cam.setPosition(0,-20,55);
--	cam.setTarget(0,0,55);

-- starisland trees
--	cam.setPosition(-311.19,-453.838,49.8557)
--	cam.setTarget(-396.103,-463.164,11.5164)

-- ocean view
	time.setCurWeather(4)
	time.setHour(10)
	time.setMinute(0)
	cam.setPosition(229.55,-1278.8,12.9011)
	cam.setTarget(272.696,-1284.95,10.8568)
--	cam.setPosition(235.227,-1247.43,16.815)
--	cam.setTarget(238.835,-1209.01,15.0051)
	renderer.setDoTrails(1)

end

if game == GTA3 then
	cam.setPosition(828.486,-557.21,33.3323)
	cam.setTarget(796.191,-595.298,30.7944)
end

-- sa start2
--cam.setTarget(2218.54,-1264.33,36.3782)
--cam.setDistance(13.0313)
--cam.setYaw(1.65411)
--cam.setPitch(4.6713)

--if game == GTASA then
--	playerLoad("cesar","cesar")
--else
--	playerLoad("player","player")
--end
--playerSetAnim("idle_stance")
--setAnim("walk_gang1")

function camanim()
	p1 = {-283.043, -470.489, 80.7301}
	t1 = {-341.418, -433.688, 17.444}

	p2 = {-1067.23, -137.067, 51.3209}
	t2 = {-1129.82, -109.211, -12.5024}

	n = 1/0.001
	dur = 10*1000*1000
	for t = 0, 1, 0.001 do
		cam.setPosition(p1[1]*(1-t) + p2[1]*t,
		                p1[2]*(1-t) + p2[2]*t,
		                p1[3]*(1-t) + p2[3]*t)
		cam.setTarget(t1[1]*(1-t) + t2[1]*t,
		              t1[2]*(1-t) + t2[2]*t,
		              t1[3]*(1-t) + t2[3]*t)
		usleep(dur/n)
	end
end

function a()
	cam.setTarget(0,0,0)
end

function b()
	cam.setTarget(-10000,-10000,-10000)
end

function c()
	setMixedAnim("walk_player", "run_player", 0.5);
end

playerSetAnim("roadcross")
