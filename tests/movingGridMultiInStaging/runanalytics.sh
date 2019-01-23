

#!/bin/bash

cat /project1/parashar-001/zw241/software/eventDrivenWorkflow/tests/movingGridInStaging/runanalytics.template | sed -e 's/TS/'$1'/' > /project1/parashar-001/zw241/software/eventDrivenWorkflow/tests/movingGridInStaging/runanalytics.scripts

slurm /project1/parashar-001/zw241/software/eventDrivenWorkflow/tests/movingGridInStaging/runanalytics.scripts
