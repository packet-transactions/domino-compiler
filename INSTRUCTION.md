1.How to run rename(rename is to rename stateful_vars to state_0, state_1 etc.)
  i.e domino_to_rename_domino "filename.c"
2.How to run grouper(grouper is to group stateful_vars together)
  i.e grouper "filename.c" "group_size(0/1)"
3.How to run domino_to_chipmunk(this is to generate .sk file from domino program)
  i.e domino_to_chipmunk "filename.c"

For example:
Step1: domino_to_rename_domino ~/chipmunk_experiment/domino_programs/avq.c > /tmp/avq.c
Step2: grouper /tmp/avq.c 2
Step3: domino_to_chipmunk /tmp/avq_equivalent_0.c
