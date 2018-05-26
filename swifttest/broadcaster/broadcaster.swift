type file;

app (file o) simulation (int sim_steps, int sim_range, int sim_values)
{
  simulate "--timesteps" sim_steps "--range" sim_range "--nvalues" sim_values stdout=filename(o);
}

app (file o) analyze (file s)
{
  stats filenames(s) stdout=filename(o);
}

int nsim   = toInt(arg("nsim","100"));
int steps  = toInt(arg("steps","1"));
int range  = toInt(arg("range","100"));
int values = toInt(arg("values","5"));


file simout <single_file_mapper; file=strcat("output/sim.out")>;
simout = simulation(steps,range,values);

foreach i in [0:nsim-1] {
  file anaout <single_file_mapper; file=strcat("output/ana_",i,".out")>;
  anaout = analyze(simout);
}
