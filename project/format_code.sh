#!/bin/bash

find .. -name *.cpp -type f -print0 | xargs -0 astyle --mode=c --style=mozilla --align-pointer=type --align-reference=type --indent=tab --indent-col1-comments --break-blocks --pad-oper --pad-comma --pad-first-paren-out --unpad-paren --pad-header --add-braces --break-return-type --break-return-type-decl --max-code-length=112 --break-after-logical --suffix=none --quiet --lineend=linux --indent-col1-comments --indent-switches --indent-cases  --indent-labels

find .. -name *.hpp -type f -print0 | xargs -0 astyle --mode=c --style=mozilla --align-pointer=type --align-reference=type --indent=tab --indent-col1-comments --break-blocks --pad-oper --pad-comma --pad-first-paren-out --unpad-paren --pad-header --add-braces --break-return-type --break-return-type-decl --max-code-length=112 --break-after-logical --suffix=none --quiet --lineend=linux --indent-col1-comments --indent-classes --indent-modifiers --indent-namespaces


