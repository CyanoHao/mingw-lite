AR      := llvm-ar
CC      := clang
CXX     := clang++
DLLTOOL := llvm-dlltool

ARCHES := i386 amd64 arm64
OBJDIR := .obj

i386_MACHINE  := i386
amd64_MACHINE := i386:x86-64
arm64_MACHINE := arm64

i386_TARGET  := i686-w64-mingw32
amd64_TARGET := x86_64-w64-mingw32
arm64_TARGET := aarch64-w64-mingw32

stdcall_def_i386  := src/def/32/stdcall.def
stdcall_def_amd64 := src/def/stdcall.def
stdcall_def_arm64 := src/def/stdcall.def

.PHONY: all clean
all: \
  $(foreach a,$(ARCHES),$(a)/libcdecl.a $(a)/libstdcall.a $(a)/libcdecl_thunk.a $(a)/libstdcall_thunk.a $(a)/libcdecl_mixed.a $(a)/libstdcall_mixed.a $(a)/libcrt.a) \
  i386/libc++rt.a amd64/libc++rt.a \
  none/notlib.a none/empty.a none/name.a

define ARCH_RULES
$(1)/libcdecl.a: src/def/cdecl.def
	$$(DLLTOOL) -m $$($(1)_MACHINE) -k -d $$< -l $$@
$(1)/libstdcall.a: $$(stdcall_def_$(1))
	$$(DLLTOOL) -m $$($(1)_MACHINE) -k -d $$< -l $$@
$(1)/libcrt.a: src/def/libcrt.def
	$$(DLLTOOL) -m $$($(1)_MACHINE) -k -d $$< -l $$@

$$(OBJDIR)/$(1)/cdecl_thunk_%.o: src/cdecl/cdecl_thunk_%.c
	@mkdir -p $$(dir $$@); $$(CC) -target $$($(1)_TARGET) -c $$< -o $$@
$$(OBJDIR)/$(1)/StdcallThunk%.o: src/stdcall/StdcallThunk%.c
	@mkdir -p $$(dir $$@); $$(CC) -target $$($(1)_TARGET) -c $$< -o $$@

$(1)/libcdecl_thunk.a: \
    $$(OBJDIR)/$(1)/cdecl_thunk_1.o $$(OBJDIR)/$(1)/cdecl_thunk_2.o $$(OBJDIR)/$(1)/cdecl_thunk_3.o
	$$(AR) rcs $$@ $$^
$(1)/libstdcall_thunk.a: \
    $$(OBJDIR)/$(1)/StdcallThunk1.o $$(OBJDIR)/$(1)/StdcallThunk2.o $$(OBJDIR)/$(1)/StdcallThunk3.o
	$$(AR) rcs $$@ $$^

$(1)/libcdecl_mixed.a: $(1)/libcdecl.a $(1)/libcdecl_thunk.a
	@printf 'CREATE %s\nADDLIB %s\nADDLIB %s\nSAVE\nEND\n' \
	  $$@ $(1)/libcdecl.a $(1)/libcdecl_thunk.a | $$(AR) -M
$(1)/libstdcall_mixed.a: $(1)/libstdcall.a $(1)/libstdcall_thunk.a
	@printf 'CREATE %s\nADDLIB %s\nADDLIB %s\nSAVE\nEND\n' \
	  $$@ $(1)/libstdcall.a $(1)/libstdcall_thunk.a | $$(AR) -M
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

none/notlib.a:
	@printf 'not an archive\n' > $@
none/empty.a:
	$(AR) rcs $@
none/name.a: $(OBJDIR)/none/simple.o $(OBJDIR)/none/object_with_very_long_name.o
	$(AR) rcs $@ $^
$(OBJDIR)/none/simple.o:
	@mkdir -p $(dir $@); printf 'simple\n' > $@
$(OBJDIR)/none/object_with_very_long_name.o:
	@mkdir -p $(dir $@); printf 'object with very long name\n' > $@

clean:
	$(RM) -r $(OBJDIR)
	$(RM) $(foreach a,$(ARCHES),$(a)/libcdecl.a $(a)/libstdcall.a $(a)/libcdecl_thunk.a $(a)/libstdcall_thunk.a $(a)/libcdecl_mixed.a $(a)/libstdcall_mixed.a $(a)/libcrt.a) i386/libc++rt.a amd64/libc++rt.a none/notlib.a none/empty.a none/name.a
