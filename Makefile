DIR = ../etc
FLAGS    = -H $(DIR)/markdown_headers.tex \
	   --highlight-style $(DIR)/code_highlighting.theme \
	   --lua-filter $(DIR)/code_snippets.lua --pdf-engine=xelatex

%.pdf: %.md
	cd $(@D) && pandoc $(FLAGS) $(<F) -o $(@F)
