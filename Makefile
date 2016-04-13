################################################################################
#                                   PROLOGUE                                   #
################################################################################

.SUFFIXES:  

# There is no default target.
all:
	@ echo -n "Currently supported target architectures are: " 1>&2
	@ echo "amd64, ia32pae, ia32" 1>&2
	@ echo "Please choose one of them as a target!" 1>&2
	@ return 42 # Yes, the answer to 'all', basically.

# Solution directories
PREFIX		:= ./build

# Common settings
include ./Beelzebub.mk

# Fake targets.
.PHONY: run qemu qemu-serial clean jegudiel image kernel apps libs $(ARC) $(SETTINGS)

# Output file
KERNEL_DIR	:= ./$(KERNEL_NAME)

################################################################################
#                             TOOLCHAIN & SETTINGS                             #
################################################################################

# Toolchain
include ./Toolchain.mk
#	This one is needed to determine Make flags.

################################################################################
#                        ARCHITECTURE-SPECIFIC SETTINGS                        #
################################################################################

##############
# 64-bit x86 #
ifeq ($(ARC),amd64)
kernel:: jegudiel
endif

################################################################################
#                                   TARGETS                                    #
################################################################################

# Do nothing for the architecture as a target.
$(ARC): image
	@ true

clean:
	@ $(MAKE) -C image/ clean
	@ $(MAKE) -C $(KERNEL_DIR)/ clean
	@ $(MAKE) -C jegudiel/ clean
	@ $(MAKE) -C apps/ clean
	@ $(MAKE) -C libs/ clean
	@ rm -Rf $(PREFIX)
	@ rm -f last_settings.txt

####################################### TESTING & RUNNING ##########

run: $(ISO_PATH)
	@ bochs -q

qemu: $(ISO_PATH)
	@ qemu-system-x86_64 -cdrom $(ISO_PATH) -smp 4

qemu-serial: $(ISO_PATH)
	@ qemu-system-x86_64 -cdrom $(ISO_PATH) -smp 4 -nographic

vmware:
	@ #echo a | /cygdrive/c/Users/rada/Dropbox/Projects/Named\ Pipe\ Server/Named\ Pipe\ Server/bin/Release/Named\ Pipe\ Server
	@ vmrun start $(VMX_PATH)

vmware2:
	@ vmrun start $(VMX_PATH)
	@ sleep 5
	@ vmrun stop $(VMX_PATH)

####################################### COMPONENTS ##########

jegudiel:
	@ echo "/MAK:" $@
	@ $(MAKE) -C jegudiel/ $(ARC) $(SETTINGS) install $(MAKE_FLAGS)

image: kernel apps libs
	@ echo "/MAK:" $@
	@ $(MAKE) -C image/ $(ARC) $(SETTINGS) iso $(MAKE_FLAGS)

kernel:: libs
	@ echo "/MAK:" $@
	@ $(MAKE) -C $(KERNEL_DIR)/ $(ARC) $(SETTINGS) install $(MAKE_FLAGS)

apps: kernel libs
	@ echo "/MAK:" $@
	@ $(MAKE) -C apps/ $(ARC) $(SETTINGS) install $(MAKE_FLAGS)

libs:
	@ echo "/MAK:" $@
	@ $(MAKE) -C libs/ $(ARC) $(SETTINGS) install $(MAKE_FLAGS)
