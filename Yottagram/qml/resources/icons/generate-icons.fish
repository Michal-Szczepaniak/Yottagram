for i in (ls *.svg); inkscape -w 86 -h 86 $i -o (string sub -e -3 $i)png; end
for i in (ls *.svg); inkscape -w 258 -h 258 $i -o +3.0/(string sub -e -3 $i)png; end
for i in (ls *.svg); inkscape -w 215 -h 215 $i -o +2.5/(string sub -e -3 $i)png; end
for i in (ls *.svg); inkscape -w 172 -h 172 $i -o +2.0/(string sub -e -3 $i)png; end
for i in (ls *.svg); inkscape -w 129 -h 129 $i -o +1.5/(string sub -e -3 $i)png; end
