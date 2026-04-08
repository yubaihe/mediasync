.PHONY: all install clean

#dirs := json-c/ sqlite-amalgamation-3310100/ pcre/ libiconv/ exiv2/
alldirs=$(shell ls -d */)
#type=0 过滤一些目录后编译 type=1 如果子目录包含out目录就不编译该子目录
type=1
ifeq ($(type), 0)
# 定义要排除的目录（需带斜杠）
	excludedirs = exiv2/ ffmpeg/
	builddirs = $(filter-out $(excludedirs), $(alldirs))
else
	builddirs = $(alldirs)
# 过滤包含out子目录的文件夹
filterdirs = $(shell \
    for d in $(1); do \
        [ ! -d "$${d%/}/out" ] && echo "$$d"; \
    done \
)
builddirs := $(call filterdirs, $(alldirs))
endif

ifneq ($(THIRDPARTLIB),)
	builddirs=$(THIRDPARTLIB)
endif

all:
	@echo "builddirs: $(builddirs)"
	$(foreach N,$(builddirs),make -C $(N) || exit $$?; make -C $(N) install || exit $$?;)
install:
	$(foreach N,$(alldirs),make -C $(N) install || exit $$?;)
clean:
	$(foreach N,$(alldirs),make -C $(N) clean || exit $$?;)
