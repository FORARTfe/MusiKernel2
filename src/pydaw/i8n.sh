find python -iname "*.py" | xargs xgettext \
--from-code=UTF-8 --default-domain=$(cat ../major-version.txt)
