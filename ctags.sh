#
# !/bin/bash
#

ctags -f ./tags -t -T -d --globals --members --declarations -R `find -name "*.c"` \
`find -name "*.h"`
