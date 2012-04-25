#! /bin/bash
#gitrev.sh by Miigotu for Wiiflow+ 2012

VERSION="Script Failed!"
STATUS=""
if $(git status 2>&1 | grep -q "fatal")
	then
		VERSION="$(date) (RAW)"
		STATUS="using the current time since there is no version control."
	else
		if ! $(git -c diff.autorefreshindex=true diff --quiet --shortstat --)
			then
				VERSION="$(date) (DIRTY)"
				STATUS="using current time since you have local modifications."
			else
				VERSION="$(git log -n 1 --pretty="format:%ad" 2>&1) (CLEAN)"
				STATUS="using last commit time since you have no local modifications."
		fi
fi

echo The version is being set as $VERSION
echo $STATUS

FILENAME=source/gitrev.h
if [ -f $FILENAME ]; then
rm $FILENAME
fi

cat <<EOF > $FILENAME
#define GIT_REV "$VERSION"
EOF
