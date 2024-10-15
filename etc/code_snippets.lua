local open = io.open
local code_classes = {}

function ImportCode(elem)
	local fname = elem.attributes['source']
	local file  = open(fname, 'r')
	if not file then return pandoc.BlockQuote(
		pandoc.CodeBlock('!!CODE_SNIPPET: Failed to load file: ' ..
		                 fname)) end

	if elem.classes:find('noNumberLines', 0) == nil then
		elem.classes:insert('numberLines')
	end
	local _,initial_lines = elem.text:gsub ('\n','\n')
	local start = tonumber(elem.attributes['startFrom'] or '1')
	local _end = tonumber(elem.attributes['endAt'])
	if elem.text == '' then
		elem.attributes['startFrom'] = tostring(start)
	else
		if string.sub(elem.text, -1) ~= '\n' then
			elem.text = elem.text .. '\n'
		end
		elem.attributes['startFrom'] = tostring(start - initial_lines - 1)
	end

	local idx = 0
	for l in file:lines('L') do
		idx = idx + 1
		if _end ~= nil and idx > _end then break end
		if idx >= start then elem.text = elem.text .. l end
	end
	file:close()
	return elem
end

function Meta(meta)
	if meta['code-classes'] then
		local tmp, from = pandoc.utils.stringify(meta['code-classes']), 1
		code_classes = pandoc.List
		repeat
			local pos = tmp:find(' ', from)
			code_classes:insert(tmp:sub(from, pos and pos-1))
			from = pos and pos + 1
		until not from
	end
end

function Code(elem)
	if #(elem.classes) == 0 then
		elem.classes = code_classes
		return elem
	end
end

function CodeBlock(elem)
	if #(elem.classes) == 0 then
		elem.classes = code_classes
		return elem
	elseif elem.attributes['source'] then
		return ImportCode(elem)
	end
end

return {
	{ Meta = Meta },
	{ Code = Code, CodeBlock = CodeBlock }
}
