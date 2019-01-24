

#!/bin/bash

PREFIX=/project1/parashar-001/zw241/software/eventDrivenWorkflow/tests/movingGridMultiInStagingEvent

cat $PREFIX/runanalytics.template | sed -e 's/TS/'$1'/' > $PREFIX/runanalytics.scripts
sbatch $PREFIX/runanalytics.scripts
