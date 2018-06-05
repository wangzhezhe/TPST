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

file simout3 <single_file_mapper; file=strcat("output/simout3.out")>;
simout3 = simulation(steps,range,values,anaout2);

file anaout3 <single_file_mapper; file=strcat("output/anaout3.out")>;
anaout3 = analyze(simout3);

file simout4 <single_file_mapper; file=strcat("output/simout4.out")>;
simout4 = simulation(steps,range,values,anaout3);

file anaout4 <single_file_mapper; file=strcat("output/anaout4.out")>;
anaout4 = analyze(simout4);

file simout5 <single_file_mapper; file=strcat("output/simout5.out")>;
simout5 = simulation(steps,range,values,anaout4);

file anaout5 <single_file_mapper; file=strcat("output/anaout5.out")>;
anaout5 = analyze(simout5);

file simout6 <single_file_mapper; file=strcat("output/simout6.out")>;
simout6 = simulation(steps,range,values,anaout5);

file anaout6 <single_file_mapper; file=strcat("output/anaout6.out")>;
anaout6 = analyze(simout6);

file simout7 <single_file_mapper; file=strcat("output/simout7.out")>;
simout7 = simulation(steps,range,values,anaout6);

file anaout7 <single_file_mapper; file=strcat("output/anaout7.out")>;
anaout7 = analyze(simout7);

file simout8 <single_file_mapper; file=strcat("output/simout8.out")>;
simout8 = simulation(steps,range,values,anaout7);

file anaout8 <single_file_mapper; file=strcat("output/anaout8.out")>;
anaout8 = analyze(simout8);

file simout9 <single_file_mapper; file=strcat("output/simout9.out")>;
simout9 = simulation(steps,range,values,anaout8);

file anaout9 <single_file_mapper; file=strcat("output/anaout9.out")>;
anaout9 = analyze(simout9);

file simout10 <single_file_mapper; file=strcat("output/simout10.out")>;
simout10 = simulation(steps,range,values,anaout9);

file anaout10 <single_file_mapper; file=strcat("output/anaout10.out")>;
anaout10 = analyze(simout10);

file simout11 <single_file_mapper; file=strcat("output/simout11.out")>;
simout11 = simulation(steps,range,values,anaout10);

file anaout11 <single_file_mapper; file=strcat("output/anaout11.out")>;
anaout11 = analyze(simout11);

file simout12 <single_file_mapper; file=strcat("output/simout12.out")>;
simout12 = simulation(steps,range,values,anaout11);

file anaout12 <single_file_mapper; file=strcat("output/anaout12.out")>;
anaout12 = analyze(simout12);

file simout13 <single_file_mapper; file=strcat("output/simout13.out")>;
simout13 = simulation(steps,range,values,anaout12);

file anaout13 <single_file_mapper; file=strcat("output/anaout13.out")>;
anaout13 = analyze(simout13);

file simout14 <single_file_mapper; file=strcat("output/simout14.out")>;
simout14 = simulation(steps,range,values,anaout13);

file anaout14 <single_file_mapper; file=strcat("output/anaout14.out")>;
anaout14 = analyze(simout14);

file simout15 <single_file_mapper; file=strcat("output/simout15.out")>;
simout15 = simulation(steps,range,values,anaout14);

file anaout15 <single_file_mapper; file=strcat("output/anaout15.out")>;
anaout15 = analyze(simout15);

file simout16 <single_file_mapper; file=strcat("output/simout16.out")>;
simout16 = simulation(steps,range,values,anaout15);

file anaout16 <single_file_mapper; file=strcat("output/anaout16.out")>;
anaout16 = analyze(simout16);

file simout17 <single_file_mapper; file=strcat("output/simout17.out")>;
simout17 = simulation(steps,range,values,anaout16);

file anaout17 <single_file_mapper; file=strcat("output/anaout17.out")>;
anaout17 = analyze(simout17);

file simout18 <single_file_mapper; file=strcat("output/simout18.out")>;
simout18 = simulation(steps,range,values,anaout17);

file anaout18 <single_file_mapper; file=strcat("output/anaout18.out")>;
anaout18 = analyze(simout18);

file simout19 <single_file_mapper; file=strcat("output/simout19.out")>;
simout19 = simulation(steps,range,values,anaout18);

file anaout19 <single_file_mapper; file=strcat("output/anaout19.out")>;
anaout19 = analyze(simout19);

file simout20 <single_file_mapper; file=strcat("output/simout20.out")>;
simout20 = simulation(steps,range,values,anaout19);

file anaout20 <single_file_mapper; file=strcat("output/anaout20.out")>;
anaout20 = analyze(simout20);

file simout21 <single_file_mapper; file=strcat("output/simout21.out")>;
simout21 = simulation(steps,range,values,anaout20);

file anaout21 <single_file_mapper; file=strcat("output/anaout21.out")>;
anaout21 = analyze(simout21);

file simout22 <single_file_mapper; file=strcat("output/simout22.out")>;
simout22 = simulation(steps,range,values,anaout21);

file anaout22 <single_file_mapper; file=strcat("output/anaout22.out")>;
anaout22 = analyze(simout22);

file simout23 <single_file_mapper; file=strcat("output/simout23.out")>;
simout23 = simulation(steps,range,values,anaout22);

file anaout23 <single_file_mapper; file=strcat("output/anaout23.out")>;
anaout23 = analyze(simout23);

file simout24 <single_file_mapper; file=strcat("output/simout24.out")>;
simout24 = simulation(steps,range,values,anaout23);

file anaout24 <single_file_mapper; file=strcat("output/anaout24.out")>;
anaout24 = analyze(simout24);

file simout25 <single_file_mapper; file=strcat("output/simout25.out")>;
simout25 = simulation(steps,range,values,anaout24);

file anaout25 <single_file_mapper; file=strcat("output/anaout25.out")>;
anaout25 = analyze(simout25);

file simout26 <single_file_mapper; file=strcat("output/simout26.out")>;
simout26 = simulation(steps,range,values,anaout25);

file anaout26 <single_file_mapper; file=strcat("output/anaout26.out")>;
anaout26 = analyze(simout26);

file simout27 <single_file_mapper; file=strcat("output/simout27.out")>;
simout27 = simulation(steps,range,values,anaout26);

file anaout27 <single_file_mapper; file=strcat("output/anaout27.out")>;
anaout27 = analyze(simout27);

file simout28 <single_file_mapper; file=strcat("output/simout28.out")>;
simout28 = simulation(steps,range,values,anaout27);

file anaout28 <single_file_mapper; file=strcat("output/anaout28.out")>;
anaout28 = analyze(simout28);

file simout29 <single_file_mapper; file=strcat("output/simout29.out")>;
simout29 = simulation(steps,range,values,anaout28);

file anaout29 <single_file_mapper; file=strcat("output/anaout29.out")>;
anaout29 = analyze(simout29);

file simout30 <single_file_mapper; file=strcat("output/simout30.out")>;
simout30 = simulation(steps,range,values,anaout29);

file anaout30 <single_file_mapper; file=strcat("output/anaout30.out")>;
anaout30 = analyze(simout30);

file simout31 <single_file_mapper; file=strcat("output/simout31.out")>;
simout31 = simulation(steps,range,values,anaout30);

file anaout31 <single_file_mapper; file=strcat("output/anaout31.out")>;
anaout31 = analyze(simout31);

file simout32 <single_file_mapper; file=strcat("output/simout32.out")>;
simout32 = simulation(steps,range,values,anaout31);

file anaout32 <single_file_mapper; file=strcat("output/anaout32.out")>;
anaout32 = analyze(simout32);

file simout33 <single_file_mapper; file=strcat("output/simout33.out")>;
simout33 = simulation(steps,range,values,anaout32);

file anaout33 <single_file_mapper; file=strcat("output/anaout33.out")>;
anaout33 = analyze(simout33);

file simout34 <single_file_mapper; file=strcat("output/simout34.out")>;
simout34 = simulation(steps,range,values,anaout33);

file anaout34 <single_file_mapper; file=strcat("output/anaout34.out")>;
anaout34 = analyze(simout34);

file simout35 <single_file_mapper; file=strcat("output/simout35.out")>;
simout35 = simulation(steps,range,values,anaout34);

file anaout35 <single_file_mapper; file=strcat("output/anaout35.out")>;
anaout35 = analyze(simout35);

file simout36 <single_file_mapper; file=strcat("output/simout36.out")>;
simout36 = simulation(steps,range,values,anaout35);

file anaout36 <single_file_mapper; file=strcat("output/anaout36.out")>;
anaout36 = analyze(simout36);

file simout37 <single_file_mapper; file=strcat("output/simout37.out")>;
simout37 = simulation(steps,range,values,anaout36);

file anaout37 <single_file_mapper; file=strcat("output/anaout37.out")>;
anaout37 = analyze(simout37);

file simout38 <single_file_mapper; file=strcat("output/simout38.out")>;
simout38 = simulation(steps,range,values,anaout37);

file anaout38 <single_file_mapper; file=strcat("output/anaout38.out")>;
anaout38 = analyze(simout38);

file simout39 <single_file_mapper; file=strcat("output/simout39.out")>;
simout39 = simulation(steps,range,values,anaout38);

file anaout39 <single_file_mapper; file=strcat("output/anaout39.out")>;
anaout39 = analyze(simout39);

file simout40 <single_file_mapper; file=strcat("output/simout40.out")>;
simout40 = simulation(steps,range,values,anaout39);

file anaout40 <single_file_mapper; file=strcat("output/anaout40.out")>;
anaout40 = analyze(simout40);

file simout41 <single_file_mapper; file=strcat("output/simout41.out")>;
simout41 = simulation(steps,range,values,anaout40);

file anaout41 <single_file_mapper; file=strcat("output/anaout41.out")>;
anaout41 = analyze(simout41);

file simout42 <single_file_mapper; file=strcat("output/simout42.out")>;
simout42 = simulation(steps,range,values,anaout41);

file anaout42 <single_file_mapper; file=strcat("output/anaout42.out")>;
anaout42 = analyze(simout42);

file simout43 <single_file_mapper; file=strcat("output/simout43.out")>;
simout43 = simulation(steps,range,values,anaout42);

file anaout43 <single_file_mapper; file=strcat("output/anaout43.out")>;
anaout43 = analyze(simout43);

file simout44 <single_file_mapper; file=strcat("output/simout44.out")>;
simout44 = simulation(steps,range,values,anaout43);

file anaout44 <single_file_mapper; file=strcat("output/anaout44.out")>;
anaout44 = analyze(simout44);

file simout45 <single_file_mapper; file=strcat("output/simout45.out")>;
simout45 = simulation(steps,range,values,anaout44);

file anaout45 <single_file_mapper; file=strcat("output/anaout45.out")>;
anaout45 = analyze(simout45);

file simout46 <single_file_mapper; file=strcat("output/simout46.out")>;
simout46 = simulation(steps,range,values,anaout45);

file anaout46 <single_file_mapper; file=strcat("output/anaout46.out")>;
anaout46 = analyze(simout46);

file simout47 <single_file_mapper; file=strcat("output/simout47.out")>;
simout47 = simulation(steps,range,values,anaout46);

file anaout47 <single_file_mapper; file=strcat("output/anaout47.out")>;
anaout47 = analyze(simout47);

file simout48 <single_file_mapper; file=strcat("output/simout48.out")>;
simout48 = simulation(steps,range,values,anaout47);

file anaout48 <single_file_mapper; file=strcat("output/anaout48.out")>;
anaout48 = analyze(simout48);

file simout49 <single_file_mapper; file=strcat("output/simout49.out")>;
simout49 = simulation(steps,range,values,anaout48);

file anaout49 <single_file_mapper; file=strcat("output/anaout49.out")>;
anaout49 = analyze(simout49);

file simout50 <single_file_mapper; file=strcat("output/simout50.out")>;
simout50 = simulation(steps,range,values,anaout49);

file anaout50 <single_file_mapper; file=strcat("output/anaout50.out")>;
anaout50 = analyze(simout50);

file simout51 <single_file_mapper; file=strcat("output/simout51.out")>;
simout51 = simulation(steps,range,values,anaout50);

file anaout51 <single_file_mapper; file=strcat("output/anaout51.out")>;
anaout51 = analyze(simout51);

file simout52 <single_file_mapper; file=strcat("output/simout52.out")>;
simout52 = simulation(steps,range,values,anaout51);

file anaout52 <single_file_mapper; file=strcat("output/anaout52.out")>;
anaout52 = analyze(simout52);

file simout53 <single_file_mapper; file=strcat("output/simout53.out")>;
simout53 = simulation(steps,range,values,anaout52);

file anaout53 <single_file_mapper; file=strcat("output/anaout53.out")>;
anaout53 = analyze(simout53);

file simout54 <single_file_mapper; file=strcat("output/simout54.out")>;
simout54 = simulation(steps,range,values,anaout53);

file anaout54 <single_file_mapper; file=strcat("output/anaout54.out")>;
anaout54 = analyze(simout54);

file simout55 <single_file_mapper; file=strcat("output/simout55.out")>;
simout55 = simulation(steps,range,values,anaout54);

file anaout55 <single_file_mapper; file=strcat("output/anaout55.out")>;
anaout55 = analyze(simout55);

file simout56 <single_file_mapper; file=strcat("output/simout56.out")>;
simout56 = simulation(steps,range,values,anaout55);

file anaout56 <single_file_mapper; file=strcat("output/anaout56.out")>;
anaout56 = analyze(simout56);

file simout57 <single_file_mapper; file=strcat("output/simout57.out")>;
simout57 = simulation(steps,range,values,anaout56);

file anaout57 <single_file_mapper; file=strcat("output/anaout57.out")>;
anaout57 = analyze(simout57);

file simout58 <single_file_mapper; file=strcat("output/simout58.out")>;
simout58 = simulation(steps,range,values,anaout57);

file anaout58 <single_file_mapper; file=strcat("output/anaout58.out")>;
anaout58 = analyze(simout58);

file simout59 <single_file_mapper; file=strcat("output/simout59.out")>;
simout59 = simulation(steps,range,values,anaout58);

file anaout59 <single_file_mapper; file=strcat("output/anaout59.out")>;
anaout59 = analyze(simout59);

file simout60 <single_file_mapper; file=strcat("output/simout60.out")>;
simout60 = simulation(steps,range,values,anaout59);

file anaout60 <single_file_mapper; file=strcat("output/anaout60.out")>;
anaout60 = analyze(simout60);

file simout61 <single_file_mapper; file=strcat("output/simout61.out")>;
simout61 = simulation(steps,range,values,anaout60);

file anaout61 <single_file_mapper; file=strcat("output/anaout61.out")>;
anaout61 = analyze(simout61);

file simout62 <single_file_mapper; file=strcat("output/simout62.out")>;
simout62 = simulation(steps,range,values,anaout61);

file anaout62 <single_file_mapper; file=strcat("output/anaout62.out")>;
anaout62 = analyze(simout62);

file simout63 <single_file_mapper; file=strcat("output/simout63.out")>;
simout63 = simulation(steps,range,values,anaout62);

file anaout63 <single_file_mapper; file=strcat("output/anaout63.out")>;
anaout63 = analyze(simout63);

file simout64 <single_file_mapper; file=strcat("output/simout64.out")>;
simout64 = simulation(steps,range,values,anaout63);

file anaout64 <single_file_mapper; file=strcat("output/anaout64.out")>;
anaout64 = analyze(simout64);