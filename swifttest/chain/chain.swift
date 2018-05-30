type file;


file initout <single_file_mapper; file=strcat("output/init.out")>;

app (file o) simulation (int sim_steps, int sim_range, int sim_values, file initfile)
{
  simulate "--timesteps" sim_steps "--range" sim_range "--nvalues" sim_values stdout=filename(o);
}


app (file o) analyze (file s)
{
  stats filenames(s) stdout=filename(o);
}

int nsim   = toInt(arg("nsim","10"));
int steps  = toInt(arg("steps","1"));
int range  = toInt(arg("range","100"));
int values = toInt(arg("values","5"));


file simout1 <single_file_mapper; file=strcat("output/sim_1.out")>;
simout1 = simulation(steps,range,values,initout);

file anaout1 <single_file_mapper; file=strcat("output/ana_1.out")>;
anaout1 = analyze(simout1);


file simout2 <single_file_mapper; file=strcat("output/simout2.out")>;
simout2 = simulation(steps,range,values,anaout1);

file anaout2 <single_file_mapper; file=strcat("output/anaout2.out")>;
anaout2 = analyze(simout2);

