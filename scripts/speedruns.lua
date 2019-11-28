local map, query, result

local function CheckForDungeon(event, player, newZone, newArea)
	local tstart = 0
	local bosses_max = 0
	map = player:GetMap()
	query = "SELECT COUNT(*) FROM custom_speedruns_runs WHERE player = '"..player:GetName().."';"
	result = WorldDBQuery(query):GetInt32(0)
	if map:IsDungeon() == true and result == 0 and player:IsInGroup() == false then
		tstart = os.time()
		dungeonName = map:GetName()
		query = "SELECT max_bosses FROM custom_speedruns_dungeons WHERE name ='".. dungeonName .."';"
		bosses_max = WorldDBQuery(query):GetInt32(0)
		local bosses_left = bosses_max
		player:Say("Start: " .. tstart .. ". Total Bosses to kill: " .. bosses_max,0)
		query = "SELECT * FROM custom_speedruns_results WHERE dungeon = '"..dungeonName.."' order by time asc;"
		result = WorldDBQuery(query)
		local tbest = result:GetInt32(3)
		player:Say("Best run by player ".. result:GetString(2) ..": " .. tbest .. " seconds", 0)
		query = "INSERT INTO custom_speedruns_runs (player, dungeon, tstart, bosses_left) VALUES ('"..  player:GetName() .."', '" .. dungeonName .. "', " .. tstart .. ", "..bosses_left.." );"
		WorldDBQuery(query)
	elseif map:IsDungeon() == false and result > 0 then
		query = "SELECT * FROM custom_speedruns_runs WHERE player = '"..player:GetName().."';"
		local kills = WorldDBQuery(query):GetInt32(3)
		player:Say("Run over. Total kills: " .. kills, 0)
		query = "DELETE FROM custom_speedruns_runs WHERE player = '"..player:GetName().."';"
		WorldDBQuery(query)
	end

end


local function AddKill(event, killer, killed)
	local map = killer:GetMap()
	if map:IsDungeon() and player:IsInGroup() == false then
		query = "SELECT * FROM custom_speedruns_runs WHERE player = '"..killer:GetName().."';"
		result = WorldDBQuery(query)
		if not result ~= nil then
			local kills = result:GetInt32(3)
			local bosses_left = result:GetInt32(4)
			local score = result:GetInt32(5)
			local tstart = result:GetInt32(2)
			kills = kills + 1
			if kills % 10 == 0 then
				killer:Say("Kills: " .. kills, 0)
			end
			local dungeonName = killer:GetMap():GetName()
			query = "SELECT * FROM custom_speedruns_dungeons WHERE name ='".. dungeonName .."';"
			local dungeon = WorldDBQuery(query)
			local entries = {}
			local found = true
			for i=0, dungeon:GetInt32(1), 1 do
				if killed:GetEntry() == dungeon:GetInt32(i+2) then
					bosses_left = bosses_left - 1
					if bosses_left > 1 then
						killer:Say("Downed a boss of " .. dungeonName .. ". Time total: " .. (os.time()-tstart) .. ". " .. bosses_left .. " bosses to go.", 0)
					elseif bosses_left == 1 then
						killer:Say("Downed a boss of " .. dungeonName .. ". Time total: " .. (os.time()-tstart) .. ". " .. bosses_left .. " more boss to go.", 0)
					elseif bosses_left == 0 then
						killer:Say("Downed the endboss of " .. dungeonName .. ". Time total: " .. (os.time()-tstart) .. ".", 0)
					end
					found = true
				end
			end
			if found then
				query = "UPDATE custom_speedruns_runs SET bosses_left = " .. bosses_left .. " WHERE player = '".. killer:GetName() .."';"
				WorldDBQuery(query)
			end
			query = "UPDATE custom_speedruns_runs SET kills = "..kills.." WHERE player = '".. killer:GetName() .."';"
			WorldDBQuery(query)
			if bosses_left == 0 then
				local tend = os.time() - tstart
				killer:Say("DUNGEON ".. dungeonName .. " COMPLETE! Time: ".. tend .. ", Total kills; " .. kills,0)
				query = "SELECT time FROM custom_speedruns_results WHERE dungeon = '"..dungeonName.."' order by time asc limit 1;"
				tbest = WorldDBQuery(query):GetInt32(0)
				if tbest > tend then
					killer:Say("New speed record! You were " .. tbest-tend .." seconds faster.",0)
				else
					query = "SELECT COUNT(*) FROM custom_speedruns_results WHERE time < " .. tend .. ";"
					local rank = WorldDBQuery(query):GetInt32(0) + 1
					killer:Say("You were " .. (tend-tbest) .. " seconds slower than rank 1. You are placed rank " .. rank ..".",0)
				end
				local uquery = "INSERT INTO custom_speedruns_results (player, dungeon, time) VALUES ('" .. killer:GetName() .. "', '".. dungeonName .."', ".. tend ..");"
				WorldDBQuery(uquery)
				uquery = "DELETE FROM custom_speedruns_runs WHERE player = '"..killer:GetName().."';"
				WorldDBQuery(uquery)
				killer:GiveXP(kills*50*killer:GetLevel())
				killer:ModifyMoney(kills*50*100*killer:GetLevel())
			end
		end
	end
end

function CheckForCommand(event, player, msg, Type, lang)
	if msg == "show stats" and player:GetMap():IsDungeon() and player:IsInGroup() == false then
		query = "SELECT * FROM custom_speedruns_runs where player = '"..player:GetName().."';"
		result = WorldDBQuery(query)
		if not result ~= nil then
			player:Say("Bosses left: "..result:GetInt32(4)..", Kills: "..result:GetInt32(3)..", Time: "..(os.time() - result:GetInt32(2)), 0)
		else
			player:Say("You are not in a speedrun.", 0)
		end
	else
		player:Say("Make sure you are in a dungeon and not in a group.",0)
	end
end

RegisterPlayerEvent(27, CheckForDungeon)
RegisterPlayerEvent(7, AddKill)
RegisterPlayerEvent(18, CheckForCommand)