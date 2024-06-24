# # Copyright (C) 2018  Christian Berger
# #
# # This Source Code Form is subject to the terms of the Mozilla Public
# # License, v. 2.0. If a copy of the MPL was not distributed with this
# # file, You can obtain one at http://mozilla.org/MPL/2.0/.

# all: run

# cluonDataStructures.proto: cluonDataStructures.odvd
# 	cluon-msc --proto --out=cluonDataStructures.proto cluonDataStructures.odvd

# cluonDataStructures_pb2.py: cluonDataStructures.proto
# 	protoc --python_out=. cluonDataStructures.proto

# MyExampleMessageSpec.proto: MyExampleMessageSpec.odvd
# 	cluon-msc --proto --out=MyExampleMessageSpec.proto MyExampleMessageSpec.odvd

# MyExampleMessageSpec_pb2.py: MyExampleMessageSpec.proto
# 	protoc --python_out=. MyExampleMessageSpec.proto

# run: cluonDataStructures_pb2.py MyExampleMessageSpec_pb2.py
# 	python receiveEnvelopes.py &
# 	sleep 2 && python sendEnvelopes.py

# clean:
# 	rm -f *_pb2.py *pyc *.proto 
all: cluonDataStructures_pb2.py opendlv_standard_message_set_v0_9_9_pb2.py MyExampleMessageSpec_pb2.py


cluonDataStructures.proto: cluonDataStructures.odvd
	cluon-msc --proto --out=cluonDataStructures.proto cluonDataStructures.odvd

cluonDataStructures_pb2.py: cluonDataStructures.proto
	protoc --python_out=. cluonDataStructures.proto


opendlv_standard_message_set_v0_9_9.proto: opendlv-standard-message-set-v0.9.9.odvd
	cluon-msc --proto --out=opendlv_standard_message_set_v0_9_9.proto opendlv-standard-message-set-v0.9.9.odvd

opendlv_standard_message_set_v0_9_9_pb2.py: opendlv_standard_message_set_v0_9_9.proto
	protoc --python_out=. opendlv_standard_message_set_v0_9_9.proto


MyExampleMessageSpec.proto: MyExampleMessageSpec.odvd
	cluon-msc --proto --out=MyExampleMessageSpec.proto MyExampleMessageSpec.odvd

MyExampleMessageSpec_pb2.py: MyExampleMessageSpec.proto
	protoc --python_out=. MyExampleMessageSpec.proto


clean:
	rm -f *_pb2.py *pyc *.proto