# How to generate chipmunk program specs from domino examples

```shell
domino_to_rename_domino ~/chipmunk_experiment/domino_programs/avq.c > /tmp/avq.c
grouper /tmp/avq.c 2
domino_to_chipmunk /tmp/avq_equivalent_0.c
```
