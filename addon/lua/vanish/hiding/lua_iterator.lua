-- The iterator forms of the roster functions. They are what addons written
-- since 2023 use instead of GetAll, and an unfiltered iterator hands out every
-- vanished player no matter how well the list functions are masked.
--
-- Both rebuild from the already masked list rather than wrapping the engine
-- iterator, so filtering stays in one place.

if Vanish.RealPlayerIterator then
	---@return function iterator, table list, number index
	function player.Iterator()
		return ipairs(player.GetAll())
	end
end

if Vanish.RealEntsIterator then
	---@return function iterator, table list, number index
	function ents.Iterator()
		return ipairs(ents.GetAll())
	end
end
