typedef struct sample{
   char Name[64];
   char Role[64];
}sampleObject;

extern "C" int function(sampleObject *s);
