del snapease.zip
cd ..

zip -X9r snapease\snapease.zip snapease -i *.cpp *.h *.xml *.txt *.ds? *.rc *.ico *.icns *.png *.strings *.xib *.pbxproj *_Prefix.pch  *.sh *.m *.plist
zip -X9r snapease\snapease.zip snapease\snapease\stage_DS_Store


cd snapease
