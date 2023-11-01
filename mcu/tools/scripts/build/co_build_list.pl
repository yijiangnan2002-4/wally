use strict;
use File::Path;
use Data::Dumper;
use YAML::Tiny;

my $counter = 0;
my $whereFileIs = `find . -name \"mapping_proj.cfg\"`;
chomp($whereFileIs);
#print "\$whereFileIs : $whereFileIs \n";
#print "command : cat ${whereFileIs} |grep \"^map__\" \n";
my $features = `cat ${whereFileIs} |grep \"^map__\"`;
$features =~ s/\r//g;
my @features = split(/\n/, $features);
`echo "#!/bin/bash" > variable_check.sh`;
`echo "rm -f err.txt" >> variable_check.sh`;
`echo "rm -f .variable_check_fail_cases" >> variable_check.sh`;
foreach my $feature(@features){

      $counter++;
      #print "\$feature : $feature\n";
      if($feature =~ /map__(.*)__(.*)=/i ){
          my $board = $1;
          my $project = $2;
          #print "\$board : $board\n";
          #print "\$project : $project\n";
          `echo "echo ${counter}. 2>&1 >> err.txt " >> variable_check.sh`;
          `echo "./build.sh $board $project -variableCompare 2>&1 >> err.txt " >> variable_check.sh`;
          `echo "echo \' \' 2>&1 >> err.txt " >> variable_check.sh`;
      }
      #if($feature !~ /\.mk/ || $feature !~ /^feature/i || $feature =~ /^feature_addon_/i || $feature =~ /switch/i || $feature =~ /full_set/i || $feature =~ /\.mk\.bak/){
      #        next;
      #}else{
      #        $feature =~ /(.*)\.mk/i;
      #        $feature = $1;
      #}

}# end of foreach

`echo 'isInFile=\$(cat err.txt | grep -c "different")' >> variable_check.sh`;
`echo 'if [ \$isInFile -eq 0 ]; then' >> variable_check.sh`;
`echo "    exit 0" >> variable_check.sh`;
`echo "else" >> variable_check.sh`;
`echo "    # keyword found" >> variable_check.sh`;
`echo "    exit 1" >> variable_check.sh`;
`echo "fi" >> variable_check.sh`;

`chmod a+x variable_check.sh`;
