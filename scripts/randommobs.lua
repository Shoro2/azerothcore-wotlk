local query, result 
local current_mob = 0

local function CheckForRng(event, instance_data, map, creature)
	if current_mob % 25 == 0 and creature:IsWorldBoss() == false then
		print(creature:GetName())
		local hp = creature:GetMaxHealth()
		creature:SetMaxHealth(hp*2)
		creature:AddAura(33719,creature)
	end
	if creature:IsWorldBoss() == false then 
		current_mob = current_mob + 1
	end
end

local function GenerateInstance(instance_id)
	RegisterInstanceEvent(5,instance_id, CheckForRng)
end

local function GetInstanceId(event, player)
	local map = player:GetMap()
	local dungeonName = map:GetName()
	query = "SELECT max_bosses FROM custom_speedruns_dungeons WHERE name='".. dungeonName .."';"
	result = WorldDBQuery(query):GetInt32(0)
	if result > 0 then
		local instance_id = map:GetInstanceId()
		GenerateInstance(instance_id)
	end
	
end

RegisterPlayerEvent(28, GetInstanceId)




