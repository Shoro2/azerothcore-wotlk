-- Command "bank" opens the player's bank  ---salkz---
 
local MSG_BANK = "#bank"
 
local function BanksOnChat(event, player, message, type, lang)
    if (message:sub(1, MSG_BANK:len()) == MSG_BANK) then
    	player:SendNotification("Opening bank")
        player:SendShowBank(player)
        return false;
    end
end
 
RegisterPlayerEvent(18, BanksOnChat)