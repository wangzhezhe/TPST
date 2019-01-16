

#!/bin/bash

module load python/3.6.3
cd /home1/zw241/pythonWorkSpace/nbodysimulation
source ./bin/activate

cd /project1/parashar-001/zw241/software/eventDrivenWorkflow/tests/movingGridInStaging/
python ./analytics.py $1