#!/usr/bin/env python2

# Copyright (C) 2018  Christian Berger
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This is the Python version of libcluon's OD4Session:
import OD4Session
import time
import joblib
import pandas as pd

# This is our example message specification.
import opendlv_standard_message_set_v0_9_9_pb2
import MyExampleMessageSpec_pb2

model = joblib.load("./ao_model.pkl")
scaler = joblib.load("./ao_scaler.pkl")
imputer = joblib.load("./ao_imputer.pkl")


# Callback for a message of interest.
def onMessage(msg, timeStamps):

    data = {
        "angularVelocityX": msg.angularVelocityX,
        "angularVelocityY": msg.angularVelocityY,
        "angularVelocityZ": msg.angularVelocityZ,
    }

    df = pd.DataFrame(data, index=[0])
    df_scaled = scaler.transform(df)
    df_imputed = imputer.transform(df_scaled)

    prediction = model.predict(df_imputed)
    print("Predicted steering:", prediction)

    steeringCMD = MyExampleMessageSpec_pb2.my_TestMessage()
    steeringCMD.steering = prediction[0]

    session.send(1234, steeringCMD.SerializeToString())


# "Main" part.
session = OD4Session.OD4Session(cid=253)  # Connect to running OD4Session at CID 253.
session.registerMessageCallback(
    1031,
    onMessage,
    opendlv_standard_message_set_v0_9_9_pb2.opendlv_proxy_AngularVelocityReading,
)


session.connect()
session.run()
