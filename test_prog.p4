field_list l3_hash_fields {
    meta.src;
    meta.dst;
}

field_list_calculation ecmp_hash {
    input {
        l3_hash_fields;
    }
    algorithm : crc16;
    output_width : 32;
}


action set_ecmp_select() {
    modify_field_with_hash_based_offset(meta.bla, 0,
                                        ecmp_hash, 10);
}

header_type meta_t {
    fields {
        bit<32> src;
        bit<32> dst;
        bit<32> bla;
        bit<32> blo;
        bit<32> tmp00;
        bit<32> tmp1;
    }
}

metadata meta_t meta;

register pp {
    width: 16;
    instance_count: 64;
}

parser start {
    return ingress;
}

action one_action(in bit<16> a, in bit<32> idx) {
    meta.bla = (1);
    meta.bla = 10 + 23 + a + (9 << (+2));
    meta.tmp00 = meta.tmp1 > 5;
    pp[idx] = meta.bla > 2 ? 16 * 78 : (bit<16>)99;
    set_ecmp_select();
}

table my_t3 {
    actions {
      one_action;
    }
}

control ingress {
    if(meta.bla == 10) {
        apply(my_t3);
    }
}
