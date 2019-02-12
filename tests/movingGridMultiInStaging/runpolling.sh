#!/bin/bash

python analytic_polling.py 300 > anap.log &

python semiEulerGridInStaging.py 300 50 