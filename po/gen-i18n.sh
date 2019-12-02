#!/bin/sh

### sh gen-i18n.sh

GEN_FILE="i18n.h"

echo "// I18N_H" > $GEN_FILE
echo "" >> $GEN_FILE

for file in *.po
do

LANG=`echo $file | sed 's|\.po||'`

echo "MsgIdStr" $LANG"_msgidstr_n[] =" >> $GEN_FILE
echo "{" >> $GEN_FILE

	MSG=`grep -E "(msgid|msgstr)" $file | grep -v "#"`
	STR=`echo $MSG | sed 's|msgid |},\n    { |g;s| msgstr |, |g'`
	echo "$STR }" >> $GEN_FILE

echo "};" >> $GEN_FILE
echo "" >> $GEN_FILE

done


echo "Langs langs_n[] =" >> $GEN_FILE
echo "{" >> $GEN_FILE

for lang in *.po
do
	STR=`echo $lang | sed 's|\.po||'`
	LNG=`cat $lang | grep "Language-Team:" | awk '{ print $2 }' | sed 's|\\\n"$||'`
	echo $LNG
	echo "    { \"$STR\", \"$LNG\", " $STR"_msgidstr_n, G_N_ELEMENTS (" $STR"_msgidstr_n ) }," >> $GEN_FILE
done

echo "};" >> $GEN_FILE
echo "" >> $GEN_FILE


sed '/^},$/d;/^ *{ "", "" },$/d' -i $GEN_FILE

