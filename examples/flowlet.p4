// header definition
header_type pkt_t {
  fields{
    bit<32> arrival;
    bit<32> dport;
    bit<32> id;
    bit<32> id0;
    bit<32> id00;
    bit<32> id1;
    bit<32> id10;
    bit<32> last_time0;
    bit<32> last_time00;
    bit<32> last_time01;
    bit<32> new_hop;
    bit<32> new_hop0;
    bit<32> next_hop;
    bit<32> next_hop0;
    bit<32> saved_hop0;
    bit<32> saved_hop00;
    bit<32> saved_hop01;
    bit<32> sport;
    bit<32> tmp0;
    bit<32> tmp00;
    bit<32> tmp1;
  }
}
header pkt_t pkt;

// parser definition 
parser start {
  return ingress;
}

// register definitions 
register last_time { width : 32; instance_count : 8000;}
register saved_hop { width : 32; instance_count : 8000;}

// primitive action definitions
field_list hash_fields_0{
pkt.sport;
pkt.dport;
pkt.arrival;
}
field_list_calculation hash_0{ input { hash_fields_0; } algorithm : crc16; output_width : 32;}
action _atom_0_2() {modify_field_with_hash_based_offset(pkt.new_hop0, 0, hash_0, 10);}

field_list hash_fields_1{
pkt.sport;
pkt.dport;
}
field_list_calculation hash_1{ input { hash_fields_1; } algorithm : crc16; output_width : 32;}
action _atom_0_1() {modify_field_with_hash_based_offset(pkt.id10, 0, hash_1, 8000);}

field_list hash_fields_2{
pkt.sport;
pkt.dport;
}
field_list_calculation hash_2{ input { hash_fields_2; } algorithm : crc16; output_width : 32;}
action _atom_0_0() {modify_field_with_hash_based_offset(pkt.id00, 0, hash_2, 8000);}

action _atom_1_0() {pkt.last_time00=last_time[pkt.id10];last_time[pkt.id10]=pkt.arrival;}

action _atom_2_0() {pkt.tmp1=pkt.arrival-pkt.last_time00;}

action _atom_3_0() {pkt.tmp00=pkt.tmp1>5;}

action _atom_4_0() {pkt.saved_hop00=saved_hop[pkt.id00];saved_hop[pkt.id00]=((pkt.tmp00)!=0) ? (pkt.new_hop0) : pkt.saved_hop00;}

action _atom_5_1() {pkt.next_hop0=((pkt.tmp00)!=0) ? (pkt.new_hop0) : pkt.saved_hop00;}

action _atom_5_0() {pkt.saved_hop01=((pkt.tmp00)!=0) ? (pkt.new_hop0) : pkt.saved_hop00;}


// table definitions
action action_stage_0() {
_atom_0_2();
_atom_0_1();
_atom_0_0();
}
table table_stage_0{ actions { action_stage_0;}}

action action_stage_1() {
_atom_1_0();
}
table table_stage_1{ actions { action_stage_1;}}

action action_stage_2() {
_atom_2_0();
}
table table_stage_2{ actions { action_stage_2;}}

action action_stage_3() {
_atom_3_0();
}
table table_stage_3{ actions { action_stage_3;}}

action action_stage_4() {
_atom_4_0();
}
table table_stage_4{ actions { action_stage_4;}}

action action_stage_5() {
_atom_5_1();
_atom_5_0();
}
table table_stage_5{ actions { action_stage_5;}}


// control program
control ingress{  apply(table_stage_0);
  apply(table_stage_1);
  apply(table_stage_2);
  apply(table_stage_3);
  apply(table_stage_4);
  apply(table_stage_5);
}