# sux makefile
#
SUBDIRS=src
ME=sux
RM=/bin/rm

all:	build

build:
	@for dir in $(SUBDIRS); do \
		echo "Building $$dir"; \
		cd $$dir; \
		make all; \
		cd ..; \
	done;
	
	@ln src/$(ME) .

	@echo "********************************";
	@echo "* suuuuuuuuuuuuuuuuuuuuuuuxxx! *";
	@echo "********************************";
	@ls -l $(ME);

clean:
	$(RM) -f $(ME) core $(ME).core;
	@for dir in $(SUBDIRS); do \
		echo "Cleaning $$dir"; \
		cd $$dir; \
		make clean; \
		cd ..; \
	done;

moo:
	@echo "         (__) "
	@echo "         (oo) "
	@echo "   /------\/ "
	@echo "  / |    ||   "
	@echo " *  /\---/\ "
	@echo "    ~~   ~~   "
	@echo "....`Have you mooed today?'..."
