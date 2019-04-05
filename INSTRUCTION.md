# How to generate chipmunk program specs from domino examples

```shell
canonicalizer ~/chipmunk_experiment/domino_programs/avq.c > /tmp/avq.c
grouper /tmp/avq.c 2
domino_to_chipmunk /tmp/avq_equivalent_0.c
```

# Combine generation chipmunk program specs from domino examples to one file
```shell
python3 compile_with_chipmunk.py ~/chipmunk_experiment/domino_programs/avq.c 2
```

Note: Now all programs in ~/chipmunk_experiment/domino_programs can run with compile_with_chipmunk.py except codel.c dns_ttl_change.c and hula_probe.c

@XiangyuG will fixe the bug later
