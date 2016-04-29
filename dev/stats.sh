#!/bin/bash -e

STATS_FILE="stats.md"

cd ..

SRC_SIZE=`du -bsh src | awk '{print $1}'`
DATA_SIZE=`du -bsh data | awk '{print $1}'`
GFX_SIZE=`du -bsh gfx | awk '{print $1}'`
SND_SIZE=`du -bsh sound | awk '{print $1}'`
MUSIC_SIZE=`du -bsh music | awk '{print $1}'`

SRC_LINES=`find src -name *.c | xargs wc -l | grep total | awk '{print $1}'`

NUM_MISSIONS=`find data/missions -iname *.json | wc -l`
NUM_CHALLENGES=`find data -name '*.json' -print0 | xargs -0 cat | grep "\"type\" : \"CHALLENGE_" | wc -l`
NUM_TROPHIES=`cat data/trophies/trophies.json | grep \"description\" | wc -l`

NUM_CRAFT=`find data/craft/* | wc -l`
NUM_FIGHTERS=`find data/fighters/* | wc -l`
NUM_GUNS=`find data/turrets/* | wc -l`
NUM_CAPS=`find data/capitalShips/* | wc -l`
TOTAL_CRAFT=$(($NUM_CRAFT + $NUM_FIGHTERS + $NUM_GUNS + $NUM_CAPS))

REVISIONS=`git rev-list --all --count`

LOCALE=`cat locale/tbftss.pot | grep msgid | wc -l`

echo "# Statistics" > $STATS_FILE
echo "" >> $STATS_FILE

echo "* Lines of code: $SRC_LINES" >> $STATS_FILE
echo "* Size of source: $SRC_SIZE" >> $STATS_FILE
echo "* Size of data: $DATA_SIZE" >> $STATS_FILE
echo "* Size of graphics: $GFX_SIZE" >> $STATS_FILE
echo "* Size of sound: $SND_SIZE" >> $STATS_FILE
echo "* Size of music: $MUSIC_SIZE" >> $STATS_FILE
echo "" >> $STATS_FILE

echo "* Number of missions: $NUM_MISSIONS" >> $STATS_FILE
echo "* Number of challenges: $NUM_CHALLENGES" >> $STATS_FILE
echo "* Number of spacecraft: $TOTAL_CRAFT" >> $STATS_FILE
echo "* Number of trophies: $NUM_TROPHIES" >> $STATS_FILE
echo "" >> $STATS_FILE

echo "* Translatable strings: $LOCALE" >> $STATS_FILE
echo "" >> $STATS_FILE

echo "* Number of GIT revisions: $REVISIONS" >> $STATS_FILE
