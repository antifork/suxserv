# sux makefile
#
SUBDIRS=src
ME=sux
RM=/bin/rm

all:	build $(ME)

build:
	@for dir in $(SUBDIRS); do \
		echo "Building $$dir"; \
		cd $$dir; \
		make all; \
		cd ..; \
	done;
	
$(ME): src/$(ME)
	@ln -f src/$(ME) .
	@ln -fs $(ME) a.out

	@echo "********************************";
	@echo "* suuuuuuuuuuuuuuuuuuuuuuuxxx! *";
	@echo "********************************";
	@ls -l $(ME);
clean:
	$(RM) -f $(ME) core core.* $(ME).core .nfs* a.out;
	@for dir in $(SUBDIRS); do \
		echo "Cleaning $$dir"; \
		cd $$dir; \
		make clean; \
		cd ..; \
	done;
	@rm -f gmon.out

moo:
	@echo "         (__) "
	@echo "         (oo) "
	@echo "   /------\/ "
	@echo "  / |    ||   "
	@echo " *  /\---/\ "
	@echo "    ~~   ~~   "
	@echo "....'Have you mooed today?'..."
