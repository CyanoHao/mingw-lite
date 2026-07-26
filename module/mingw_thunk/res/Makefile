AR      := llvm-ar
CC      := clang
CXX     := clang++
DLLTOOL := llvm-dlltool

ARCHES  := i386 amd64 arm64
OBJDIR  := .obj

i386_MACHINE  := i386
amd64_MACHINE := i386:x86-64
arm64_MACHINE := arm64

i386_TARGET  := i686-w64-mingw32
amd64_TARGET := x86_64-w64-mingw32
arm64_TARGET := aarch64-w64-mingw32

i386_DEFPREFIX  := 32/
amd64_DEFPREFIX :=
arm64_DEFPREFIX :=

arm64_CFLAGS    := -isystem /usr/x86_64-w64-mingw32/include

KERNEL32_SRCS := WideCharToMultiByte.c MultiByteToWideChar.c CreateFileA.c CreateFileW.c
LIBCRT_SRCS   := access.c strnicmp.c

.PHONY: all clean
all: \
  $(foreach a,$(ARCHES),$(a)/libkernel32.a $(a)/alias-kernel32.a $(a)/thunk-kernel32.a) \
  $(foreach a,$(ARCHES),$(a)/libcrt.a $(a)/alias-libcrt.a $(a)/thunk-libcrt.a) \
  i386/libc++rt.a amd64/libc++rt.a

define ARCH_RULES
$(1)/libkernel32.a: src/def/$$($(1)_DEFPREFIX)kernel32.def
	$$(DLLTOOL) -m $$($(1)_MACHINE) -k -d $$< -l $$@

$(1)/alias-kernel32.a: src/def/$$($(1)_DEFPREFIX)alias-kernel32.def
	$$(DLLTOOL) -m $$($(1)_MACHINE) -k -d $$< -l $$@

$$(OBJDIR)/$(1)/kernel32/%.o: src/kernel32/%.c
	@mkdir -p $$(dir $$@); $$(CC) -target $$($(1)_TARGET) $$($(1)_CFLAGS) -c $$< -o $$@
$(1)/thunk-kernel32.a: $$(foreach s,$$(KERNEL32_SRCS),$$(OBJDIR)/$(1)/kernel32/$$(s:.c=.o))
	$$(AR) rcs $$@ $$^

$(1)/libcrt.a: src/def/libcrt.def
	$$(DLLTOOL) -m $$($(1)_MACHINE) -k -d $$< -l $$@
$(1)/alias-libcrt.a: src/def/alias-libcrt.def
	$$(DLLTOOL) -m $$($(1)_MACHINE) -k -d $$< -l $$@

$$(OBJDIR)/$(1)/libcrt/%.o: src/libcrt/%.c
	@mkdir -p $$(dir $$@); $$(CC) -target $$($(1)_TARGET) $$($(1)_CFLAGS) -c $$< -o $$@
$(1)/thunk-libcrt.a: $$(foreach s,$$(LIBCRT_SRCS),$$(OBJDIR)/$(1)/libcrt/$$(s:.c=.o))
	$$(AR) rcs $$@ $$^
endef

$(foreach a,$(ARCHES),$(eval $(call ARCH_RULES,$(a))))

i386/libc++rt.a: src/libcxxrt/runtime.cc
	@mkdir -p $(OBJDIR)/libcxxrt
	$(CXX) -target $(i386_TARGET) -shared -o $(OBJDIR)/libcxxrt/i386.dll $< -Wl,--out-implib,$@.raw
	@printf 'CREATE %s\nADDLIB %s\nSAVE\nEND\n' $@ $@.raw | $(AR) -M
	@rm -f $@.raw
amd64/libc++rt.a: src/libcxxrt/runtime.cc
	@mkdir -p $(OBJDIR)/libcxxrt
	$(CXX) -target $(amd64_TARGET) -shared -o $(OBJDIR)/libcxxrt/amd64.dll $< -Wl,--out-implib,$@.raw
	@printf 'CREATE %s\nADDLIB %s\nSAVE\nEND\n' $@ $@.raw | $(AR) -M
	@rm -f $@.raw

clean:
	$(RM) -r $(OBJDIR)
	$(RM) $(foreach a,$(ARCHES),$(a)/libkernel32.a $(a)/alias-kernel32.a $(a)/thunk-kernel32.a $(a)/libcrt.a $(a)/alias-libcrt.a $(a)/thunk-libcrt.a) i386/libc++rt.a amd64/libc++rt.a
