# doxygen documentation configuration
DX_SRC_DIR   := . ../src/opaq_core 
DX_DOC_DIR   := .
#DX_GEN_HTML  := YES
#DX_GEN_LATEX := YES
#DX_GEN_RTF   := NO
DX_HAVE_DOT  := YES

all: html pdf rtf

html:
	$(eval DX_GEN_HTML  := YES)
	$(eval DX_GEN_LATEX := NO)
	$(eval DX_GEN_RTF   := NO)
	env PROJECT="$(PROJECT)" \
            VERSION="$(VERSION)" \
            SRCDIR="$(DX_SRC_DIR)" \
            DOCDIR="$(DX_DOC_DIR)" \
            GENERATE_HTML="$(DX_GEN_HTML)" \
            GENERATE_LATEX="$(DX_GEN_LATEX)" \
            GENERATE_RTF="$(DX_GEN_RTF)" \
            PERL_PATH=$(shell which perl) \
            HAVE_DOT="$(DX_HAVE_DOT)" \
            doxygen doxygen.cfg

rtf:
	$(eval DX_GEN_HTML  := NO)
	$(eval DX_GEN_LATEX := NO)
	$(eval DX_GEN_RTF   := YES)
	env PROJECT="$(PROJECT)" \
            VERSION="$(VERSION)" \
            SRCDIR="$(DX_SRC_DIR)" \
            DOCDIR="$(DX_DOC_DIR)" \
            GENERATE_HTML="$(DX_GEN_HTML)" \
            GENERATE_LATEX="$(DX_GEN_LATEX)" \
            GENERATE_RTF="$(DX_GEN_RTF)" \
            PERL_PATH=$(shell which perl) \
            HAVE_DOT="$(DX_HAVE_DOT)" \
            doxygen doxygen.cfg
	mv refman.rtf ../$(PROJECT)-$(VERSION)-manual.rtf

latex:
	$(eval DX_GEN_HTML  := NO)
	$(eval DX_GEN_LATEX := YES)
	$(eval DX_GEN_RTF   := NO)
	env PROJECT="$(PROJECT)" \
            VERSION="$(VERSION)" \
            SRCDIR="$(DX_SRC_DIR)" \
            DOCDIR="$(DX_DOC_DIR)" \
            GENERATE_HTML="$(DX_GEN_HTML)" \
            GENERATE_LATEX="$(DX_GEN_LATEX)" \
            GENERATE_RTF="$(DX_GEN_RTF)" \
            PERL_PATH=$(shell which perl) \
            HAVE_DOT="$(DX_HAVE_DOT)" \
            doxygen doxygen.cfg

pdf: latex
	cd $(DX_DOC_DIR)/latex; \
          rm -f *.aux *.toc *.idx *.ind *.ilg *.log *.out; \
          pdflatex refman.tex; \
          makeindex refman.idx; \
          pdflatex refman.tex; \
          countdown=5; \
          while egrep 'Rerun (LaTeX|to get cross-references right)' \
            refman.log > /dev/null 2>&1 \
              && test $$countdown -gt 0; do \
              pdflatex refman.tex; \
              countdown=`expr $$countdown - 1`; \
          done; \
          mv refman.pdf ../$(PROJECT)-$(VERSION)-manual.pdf

clean:
	rm -rf html
	rm -rf latex
	rm -rf rtf
	rm -rf $(PROJECT)-$(VERSION)-manual.rtf
	rm -rf $(PROJECT)-$(VERSION)-manual.pdf
	rm -rf *.tag
