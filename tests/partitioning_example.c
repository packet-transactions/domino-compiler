// Original program

typedef struct Packet {
  int x;
  int y;
  int z;
  int w;
} Packet;

void func(Packet p) {
  p.x = 1;
  p.y = 2;
  if (p.x == 3) {
    if (p.y == 5) {
      p.z = 3;
    } else  {
      p.w = 2;
    }
  } else {
    p.x = p.y + 23;
    p.w = p.x;
  }
}

// if-converted program

typedef struct Packet {
  int x;
  int y;
  int z;
  int w;
} Packet;

void func(Packet p) {
  int tmp__6936;
  int tmp__6702;
  int p__w;
  int p__x;
  int p__y;
  int p__z;

  p__x = 1;
  p__y = 2;
  tmp__6936 = p__x == 3;
  tmp__6702 = (tmp__6936 ? (p__y == 5) : tmp__6702);
  p__z = (tmp__6936 ? ((tmp__6702 ? (3) : p__z)) : p__z);

  p__w = (tmp__6936 ? ((!tmp__6702 ? (2) : p__w)) : p__w);

  p__x = (!tmp__6936 ? (p__y + 23) : p__x);
  p__w = (!tmp__6936 ? (p__x) : p__w);
}

// Partitioned output:

// Clock 0 :
//  { p__x = 1 }   { p__y = 2 }  
// Clock 1 : 
//  { tmp__6936 = p__x == 3 }  
// Clock 2 : 
//  { tmp__6702 = (tmp__6936 ? (p__y == 5) : tmp__6702) }   { p__x = (!tmp__6936 ? (p__y + 23) : p__x) }
// Clock 3 :
//  { p__z = (tmp__6936 ? ((tmp__6702 ? (3) : p__z)) : p__z) }   { p__w = (tmp__6936 ? ((!tmp__6702 ? (2) : p__w)) : p__w) }
// Clock 4 : 
//  { p__w = (!tmp__6936 ? (p__x) : p__w) }
