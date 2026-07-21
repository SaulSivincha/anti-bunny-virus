MAIN := main
LATEXMK := latexmk

all:
	$(LATEXMK) -pdf -interaction=nonstopmode -halt-on-error $(MAIN).tex

watch:
	$(LATEXMK) -pdf -pvc $(MAIN).tex

clean:
	$(LATEXMK) -C $(MAIN).tex

.PHONY: all watch clean
