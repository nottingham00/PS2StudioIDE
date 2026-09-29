#include "RuntimeTemplates.h"
#include <QString>
#include <QRegularExpression>

QString ps2RuntimeHeader(){return R"PS2(#pragma once
#include <tamtypes.h>
#include <gsKit.h>
#include <libpad.h>
#include <audsrv.h>
#include "PS2Assets.h"

struct PS2Transform { float x,y,z,rx,ry,rz,sx,sy,sz; };
struct PS2Entity {
    const char* name; const char* tag; int type; PS2Transform transform;
    float vx,vy,vz;
    const char* asset; const char* material; float tintR,tintG,tintB,alpha; int unlit,transparent; float lightIntensity;
    const char* animation; float animationSpeed; int animationLoop,animationAutoplay; const char* script;
    const char* uiText; const char* uiAction; float uiR,uiG,uiB,uiW,uiH; int uiSelectable; const char* nextScene;
    float colliderX,colliderY,colliderZ; int colliderEnabled,trigger,rigidBody,kinematic,useGravity; float gravityScale,mass,restitution,friction; int collisionLayer,collisionMask;
    int particleEmitter,particleCount; float particleLife;
    const char* audioAsset; int audioAutoplay,audioLoop; float audioVolume,audioPan;
};
struct PS2Scene { const char* name; const PS2Entity* entities; int count; };
enum { PS2_ENTITY_EMPTY=0,PS2_ENTITY_SPRITE=1,PS2_ENTITY_MESH=2,PS2_ENTITY_CAMERA=3,PS2_ENTITY_LIGHT=4,PS2_ENTITY_TEXT=5,PS2_ENTITY_PANEL=6,PS2_ENTITY_BUTTON=7,PS2_ENTITY_IMAGE=8,PS2_ENTITY_AUDIO=9 };
enum PS2RendererBackend { PS2_RENDERER_EE=0, PS2_RENDERER_VU1_EXPERIMENTAL=1 };

struct PS2RuntimeEntity {
    const PS2Entity* source;
    PS2Transform transform;
    float vx,vy,vz;
    float animationTime;
    int active,started,grounded,audioStarted;
};
struct PS2CollisionEvent { PS2RuntimeEntity* other; float nx,ny,nz; int trigger; };

struct PS2RuntimeStats { unsigned frame,drawCalls,triangles,entities,collisions,audioVoices; unsigned physicsPairs,skinVertices,vifPackets,vuDispatches; };
enum PS2ColliderShape { PS2_SHAPE_BOX=0, PS2_SHAPE_SPHERE=1, PS2_SHAPE_CAPSULE=2 };
struct PS2PhysicsMaterial { float friction,restitution; };
struct PS2AnimationTransition { const char* fromState; const char* toState; float blendSeconds; };
struct PS2AnimationState { const char* name; const char* clip; float speed; int loop; };
struct PS2AudioBus { const char* name; float volume; int muted; };
struct PS2UILayout { float anchorMinX,anchorMinY,anchorMaxX,anchorMaxY,pivotX,pivotY; };
class PS2AnimationStateMachine {
public:
    void bind(PS2RuntimeEntity* e){m_entity=e;} void setState(const char* name,float blend=0.15f){m_state=name;m_blend=blend;m_time=0.0f;}
    const char* state()const{return m_state;} float blend()const{return m_blend;} void update(float dt){m_time+=dt;}
private: PS2RuntimeEntity* m_entity=nullptr; const char* m_state=nullptr; float m_blend=0.0f,m_time=0.0f;
};
class PS2AudioBusMixer {
public:
    void setMaster(float v){m_master=v<0?0:(v>1?1:v);} float master()const{return m_master;}
    void setMusic(float v){m_music=v;} void setSfx(float v){m_sfx=v;} float music()const{return m_music*m_master;} float sfx()const{return m_sfx*m_master;}
private: float m_master=1.0f,m_music=1.0f,m_sfx=1.0f;
};
struct PS2VU1Backend {
    int initialized=0, verified=0; unsigned uploadQwords=0,dispatches=0;
    bool initialize(){initialized=1;return true;}
    bool dispatchTransform(const void*,unsigned){++dispatches;return false;} // false => EE fallback until VIF/DMA hardware verification
};

class PS2SceneRuntime;

// Generated component registry dispatches these calls to scripts/*.cpp.
void ps2studio_component_start(const char* script,PS2SceneRuntime* runtime,PS2RuntimeEntity* self);
void ps2studio_component_update(const char* script,PS2SceneRuntime* runtime,PS2RuntimeEntity* self,float dt);
void ps2studio_component_collision(const char* script,PS2SceneRuntime* runtime,PS2RuntimeEntity* self,const PS2CollisionEvent* ev);
void ps2studio_component_action(const char* script,PS2SceneRuntime* runtime,PS2RuntimeEntity* self,const char* action);

class PS2Input {
public: bool init(); void update(); bool down(u32 b)const{return(m_current&b)!=0;} bool pressed(u32 b)const{return(m_pressed&b)!=0;} int leftX()const{return m_lx;} int leftY()const{return m_ly;}
private:u32 m_current=0,m_previous=0,m_pressed=0;int m_lx=128,m_ly=128;bool m_ready=false;
};
class PS2AudioMixer {
public:
    bool init(const char* audsrvIrx="host:audsrv.irx");
    int playSfx(const char* asset,float volume=1.0f,float pan=0.0f);
    bool playMusic(const char* asset,bool loop=false,float volume=1.0f);
    void setMasterVolume(int v); void stopMusic(); void shutdown(); unsigned activeVoices() const{return m_activeVoices;}
private:
    const PS2EmbeddedSound* find(const char*)const; bool playPcm(const PS2EmbeddedSound*,float);
    bool m_ready=false;int m_volume=MAX_VOLUME; audsrv_adpcm_t* m_adpcm=nullptr; unsigned char* m_loaded=nullptr; unsigned m_activeVoices=0;
};
class PS2Collision { public: static bool aabb3(const PS2RuntimeEntity&,const PS2RuntimeEntity&); };

class PS2SceneRuntime {
public:
    explicit PS2SceneRuntime(GSGLOBAL* gs); ~PS2SceneRuntime();
    bool initialize(); void setRendererBackend(PS2RendererBackend b){m_backend=b;} PS2RendererBackend rendererBackend()const{return m_backend;}
    void beginFrame(u64 clearColor); void update(float dt); void drawCurrentScene(); void endFrame();
    bool loadScene(const char* name); const PS2Scene* currentScene()const{return m_scene;} int entityCount()const{return m_entityCount;}
    PS2RuntimeEntity* entity(int index); PS2RuntimeEntity* findByTag(const char* tag); PS2RuntimeEntity* findByName(const char* name);
    bool playSound(const char* asset,float volume=1.0f,float pan=0.0f){return audio.playSfx(asset,volume,pan)>=0;}
    bool switchScene(const char* name){return loadScene(name);} const PS2RuntimeStats& stats()const{return m_stats;}
    PS2Input input; PS2AudioMixer audio;
private:
    GSGLOBAL* m_gs{}; GSTEXTURE* m_textures{}; const PS2Scene* m_scene{}; PS2RuntimeEntity* m_entities{}; int m_entityCount=0; int m_uiSelected=-1; PS2RendererBackend m_backend=PS2_RENDERER_EE; PS2RuntimeStats m_stats{};
    const PS2EmbeddedTexture* findTexture(const char*,int* index=nullptr)const; const PS2EmbeddedMesh* findMesh(const char*)const; const PS2EmbeddedSkinMesh* findSkinMesh(const char*)const; const PS2EmbeddedAnimation* findAnimation(const char*)const;
    void releaseScene(); void startScene(); void updateAnimations(float dt); void updatePhysics(float dt); void updateUI();
    void dispatchCollision(PS2RuntimeEntity&,PS2RuntimeEntity&,float,float,float,int);
    void drawEntity(const PS2RuntimeEntity&); void drawSprite(const PS2RuntimeEntity&); void drawMeshEE(const PS2RuntimeEntity&); void drawMeshVU1(const PS2RuntimeEntity&); void drawSkinnedMesh(const PS2RuntimeEntity&); void drawText(const PS2RuntimeEntity&); void drawUIBox(const PS2RuntimeEntity&,bool selected); void drawParticles(const PS2RuntimeEntity&);
};
)PS2";}

QString ps2RuntimeSource(){return R"PS2(#include "PS2SceneRuntime.h"
#include "PS2SceneData.h"
#include <dmaKit.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
static char g_padBuf[256] __attribute__((aligned(64)));
static float clampf(float v,float a,float b){return v<a?a:(v>b?b:v);}static float deg2rad(float d){return d*0.01745329251994329577f;}

bool PS2Input::init(){sceSifInitRpc(0);SifLoadModule("rom0:SIO2MAN",0,nullptr);SifLoadModule("rom0:PADMAN",0,nullptr);if(padInit(0)==0)return false;m_ready=padPortOpen(0,0,g_padBuf)!=0;return m_ready;}
void PS2Input::update(){m_pressed=0;if(!m_ready)return;int st=padGetState(0,0);if(st!=PAD_STATE_STABLE&&st!=PAD_STATE_FINDCTP1)return;padButtonStatus b{};if(!padRead(0,0,&b))return;m_previous=m_current;m_current=0xffffu^b.btns;m_pressed=m_current&~m_previous;m_lx=b.ljoy_h;m_ly=b.ljoy_v;}

const PS2EmbeddedSound*PS2AudioMixer::find(const char*n)const{if(!n)return nullptr;for(int i=0;i<g_ps2SoundCount;i++)if(g_ps2Sounds[i].name&&strcmp(g_ps2Sounds[i].name,n)==0)return &g_ps2Sounds[i];return nullptr;}
bool PS2AudioMixer::init(const char*irx){sceSifInitRpc(0);SifLoadModule("rom0:LIBSD",0,nullptr);if(irx)SifLoadModule(irx,0,nullptr);m_ready=(audsrv_init()==0);if(!m_ready)return false;audsrv_set_volume(m_volume);audsrv_adpcm_init();if(g_ps2SoundCount>0){m_adpcm=(audsrv_adpcm_t*)calloc(g_ps2SoundCount,sizeof(audsrv_adpcm_t));m_loaded=(unsigned char*)calloc(g_ps2SoundCount,1);for(int i=0;i<g_ps2SoundCount;i++){const auto&s=g_ps2Sounds[i];if(s.encoding==PS2_AUDIO_ADPCM&&s.data&&s.bytes>16){if(audsrv_load_adpcm(&m_adpcm[i],(void*)s.data,s.bytes)==0)m_loaded[i]=1;}}}return true;}
void PS2AudioMixer::setMasterVolume(int v){m_volume=v<0?0:(v>MAX_VOLUME?MAX_VOLUME:v);if(m_ready)audsrv_set_volume(m_volume);}
bool PS2AudioMixer::playPcm(const PS2EmbeddedSound*s,float volume){if(!m_ready||!s||s->encoding!=PS2_AUDIO_PCM16)return false;audsrv_fmt_t f{};f.bits=s->bits;f.freq=s->rate;f.channels=s->channels;if(audsrv_set_format(&f)!=0)return false;audsrv_set_volume((int)(clampf(volume,0,1)*m_volume));if(audsrv_wait_audio(s->bytes)!=0)return false;return audsrv_play_audio((const char*)s->data,s->bytes)>=0;}
int PS2AudioMixer::playSfx(const char*n,float volume,float pan){if(!m_ready||!n)return-1;for(int i=0;i<g_ps2SoundCount;i++){const auto&s=g_ps2Sounds[i];if(!s.name||strcmp(s.name,n)!=0)continue;if(s.encoding==PS2_AUDIO_ADPCM&&m_loaded&&m_loaded[i]){int ch=audsrv_ch_play_adpcm(-1,&m_adpcm[i]);if(ch>=0){int vol=(int)(clampf(volume,0,1)*MAX_VOLUME);int p=(int)(clampf(pan,-1,1)*MAX_VOLUME);audsrv_adpcm_set_volume_and_pan(ch,vol,p);m_activeVoices++;}return ch;}return playPcm(&s,volume)?0:-1;}return-1;}
bool PS2AudioMixer::playMusic(const char*n,bool loop,float volume){(void)loop;auto*s=find(n);if(!s)return false;if(s->encoding==PS2_AUDIO_ADPCM)return playSfx(n,volume,0)>=0;return playPcm(s,volume);}
void PS2AudioMixer::stopMusic(){if(m_ready)audsrv_stop_audio();}
void PS2AudioMixer::shutdown(){if(m_ready)audsrv_quit();m_ready=false;if(m_adpcm)free(m_adpcm);if(m_loaded)free(m_loaded);m_adpcm=nullptr;m_loaded=nullptr;}

bool PS2Collision::aabb3(const PS2RuntimeEntity&a,const PS2RuntimeEntity&b){const auto&A=*a.source;const auto&B=*b.source;float ahx=A.colliderX*.5f,ahy=A.colliderY*.5f,ahz=A.colliderZ*.5f,bhx=B.colliderX*.5f,bhy=B.colliderY*.5f,bhz=B.colliderZ*.5f;return fabsf(a.transform.x-b.transform.x)<ahx+bhx&&fabsf(a.transform.y-b.transform.y)<ahy+bhy&&fabsf(a.transform.z-b.transform.z)<ahz+bhz;}

struct M4{float m[16];};
static M4 ident4(){M4 r={};r.m[0]=r.m[5]=r.m[10]=r.m[15]=1;return r;}
static M4 mul4(const M4&a,const M4&b){M4 r={};for(int y=0;y<4;y++)for(int x=0;x<4;x++)for(int k=0;k<4;k++)r.m[y*4+x]+=a.m[y*4+k]*b.m[k*4+x];return r;}
static M4 trs4(float x,float y,float z,float rx,float ry,float rz,float sx,float sy,float sz){float cx=cosf(deg2rad(rx)),sxv=sinf(deg2rad(rx)),cy=cosf(deg2rad(ry)),syv=sinf(deg2rad(ry)),cz=cosf(deg2rad(rz)),szv=sinf(deg2rad(rz));M4 S=ident4();S.m[0]=sx;S.m[5]=sy;S.m[10]=sz;M4 X=ident4();X.m[5]=cx;X.m[6]=-sxv;X.m[9]=sxv;X.m[10]=cx;M4 Y=ident4();Y.m[0]=cy;Y.m[2]=syv;Y.m[8]=-syv;Y.m[10]=cy;M4 Z=ident4();Z.m[0]=cz;Z.m[1]=-szv;Z.m[4]=szv;Z.m[5]=cz;M4 T=ident4();T.m[3]=x;T.m[7]=y;T.m[11]=z;return mul4(T,mul4(Z,mul4(Y,mul4(X,S))));}
static void transformPoint(const M4&m,float x,float y,float z,float&ox,float&oy,float&oz){ox=m.m[0]*x+m.m[1]*y+m.m[2]*z+m.m[3];oy=m.m[4]*x+m.m[5]*y+m.m[6]*z+m.m[7];oz=m.m[8]*x+m.m[9]*y+m.m[10]*z+m.m[11];}
static M4 fromArray(const float*v){M4 r;for(int i=0;i<16;i++)r.m[i]=v[i];return r;}
static PS2AnimKey sampleKeys(const PS2AnimKey*k,int n,float t){if(n<=0){PS2AnimKey z={};z.sx=z.sy=z.sz=1;return z;}if(n==1||t<=k[0].time)return k[0];if(t>=k[n-1].time)return k[n-1];for(int i=0;i<n-1;i++)if(t>=k[i].time&&t<=k[i+1].time){float d=k[i+1].time-k[i].time;float a=d>0?(t-k[i].time)/d:0;PS2AnimKey r=k[i];float*rp=&r.px;const float*ap=&k[i].px;const float*bp=&k[i+1].px;for(int q=0;q<9;q++)rp[q]=ap[q]+(bp[q]-ap[q])*a;r.time=t;return r;}return k[n-1];}

PS2SceneRuntime::PS2SceneRuntime(GSGLOBAL*gs):m_gs(gs){}
PS2SceneRuntime::~PS2SceneRuntime(){releaseScene();audio.shutdown();if(m_textures)free(m_textures);}
bool PS2SceneRuntime::initialize(){if(!m_gs)return false;input.init();audio.init();if(g_ps2TextureCount>0){m_textures=(GSTEXTURE*)calloc(g_ps2TextureCount,sizeof(GSTEXTURE));for(int i=0;i<g_ps2TextureCount;i++){const auto&a=g_ps2Textures[i];auto&t=m_textures[i];t.Width=a.width;t.Height=a.height;t.PSM=GS_PSM_CT32;t.Filter=GS_FILTER_LINEAR;t.Mem=(void*)a.rgba;t.Vram=gsKit_vram_alloc(m_gs,gsKit_texture_size(t.Width,t.Height,t.PSM),GSKIT_ALLOC_USERBUFFER);gsKit_texture_upload(m_gs,&t);}}gsKit_set_clamp(m_gs,GS_CMODE_CLAMP);if(g_sceneCount>0)return loadScene(g_scenes[0].name);return true;}
void PS2SceneRuntime::releaseScene(){if(m_entities)free(m_entities);m_entities=nullptr;m_entityCount=0;m_scene=nullptr;m_uiSelected=-1;}
bool PS2SceneRuntime::loadScene(const char*n){if(!n)return false;const PS2Scene*target=nullptr;for(int i=0;i<g_sceneCount;i++)if(g_scenes[i].name&&strcmp(g_scenes[i].name,n)==0){target=&g_scenes[i];break;}if(!target)return false;releaseScene();m_scene=target;m_entityCount=target->count;if(m_entityCount>0){m_entities=(PS2RuntimeEntity*)calloc(m_entityCount,sizeof(PS2RuntimeEntity));for(int i=0;i<m_entityCount;i++){auto&r=m_entities[i];r.source=&target->entities[i];r.transform=r.source->transform;r.vx=r.source->vx;r.vy=r.source->vy;r.vz=r.source->vz;r.active=1;r.animationTime=0;if(m_uiSelected<0&&r.source->uiSelectable)m_uiSelected=i;}}startScene();return true;}
void PS2SceneRuntime::startScene(){for(int i=0;i<m_entityCount;i++){auto&e=m_entities[i];if(!e.active)continue;if(e.source->script&&*e.source->script){ps2studio_component_start(e.source->script,this,&e);e.started=1;}if(e.source->audioAutoplay&&e.source->audioAsset&&!e.audioStarted){audio.playSfx(e.source->audioAsset,e.source->audioVolume,e.source->audioPan);e.audioStarted=1;}}}
PS2RuntimeEntity*PS2SceneRuntime::entity(int i){return i>=0&&i<m_entityCount?&m_entities[i]:nullptr;}
PS2RuntimeEntity*PS2SceneRuntime::findByTag(const char*t){if(!t)return nullptr;for(int i=0;i<m_entityCount;i++)if(m_entities[i].active&&m_entities[i].source->tag&&strcmp(m_entities[i].source->tag,t)==0)return&m_entities[i];return nullptr;}
PS2RuntimeEntity*PS2SceneRuntime::findByName(const char*t){if(!t)return nullptr;for(int i=0;i<m_entityCount;i++)if(m_entities[i].active&&m_entities[i].source->name&&strcmp(m_entities[i].source->name,t)==0)return&m_entities[i];return nullptr;}
const PS2EmbeddedTexture*PS2SceneRuntime::findTexture(const char*n,int*idx)const{if(!n)return nullptr;for(int i=0;i<g_ps2TextureCount;i++)if(g_ps2Textures[i].name&&strcmp(g_ps2Textures[i].name,n)==0){if(idx)*idx=i;return&g_ps2Textures[i];}return nullptr;}
const PS2EmbeddedMesh*PS2SceneRuntime::findMesh(const char*n)const{if(!n)return nullptr;for(int i=0;i<g_ps2MeshCount;i++)if(g_ps2Meshes[i].name&&strcmp(g_ps2Meshes[i].name,n)==0)return&g_ps2Meshes[i];return nullptr;}
const PS2EmbeddedSkinMesh*PS2SceneRuntime::findSkinMesh(const char*n)const{if(!n)return nullptr;for(int i=0;i<g_ps2SkinMeshCount;i++)if(g_ps2SkinMeshes[i].name&&strcmp(g_ps2SkinMeshes[i].name,n)==0)return&g_ps2SkinMeshes[i];return nullptr;}
const PS2EmbeddedAnimation*PS2SceneRuntime::findAnimation(const char*n)const{if(!n)return nullptr;for(int i=0;i<g_ps2AnimationCount;i++)if(g_ps2Animations[i].name&&strcmp(g_ps2Animations[i].name,n)==0)return&g_ps2Animations[i];return nullptr;}
void PS2SceneRuntime::beginFrame(u64 c){input.update();m_stats.drawCalls=m_stats.triangles=m_stats.entities=m_stats.collisions=0;m_stats.audioVoices=audio.activeVoices();gsKit_clear(m_gs,c);}
void PS2SceneRuntime::updateUI(){if(m_entityCount<=0)return;if(input.pressed(PAD_UP)||input.pressed(PAD_DOWN)){int dir=input.pressed(PAD_UP)?-1:1;int start=m_uiSelected<0?0:m_uiSelected;for(int s=1;s<=m_entityCount;s++){int i=(start+dir*s+m_entityCount)%m_entityCount;if(m_entities[i].active&&m_entities[i].source->uiSelectable){m_uiSelected=i;break;}}}if(m_uiSelected>=0&&input.pressed(PAD_CROSS)){auto&e=m_entities[m_uiSelected];if(e.source->script&&*e.source->script)ps2studio_component_action(e.source->script,this,&e,e.source->uiAction?e.source->uiAction:"");if(e.source->nextScene&&*e.source->nextScene)loadScene(e.source->nextScene);}}
void PS2SceneRuntime::updateAnimations(float dt){for(int i=0;i<m_entityCount;i++){auto&e=m_entities[i];if(!e.active||!e.source->animationAutoplay||!e.source->animation)continue;auto*a=findAnimation(e.source->animation);if(!a||a->duration<=0)continue;e.animationTime+=dt*e.source->animationSpeed;if(e.source->animationLoop||a->loop){while(e.animationTime>=a->duration)e.animationTime-=a->duration;while(e.animationTime<0)e.animationTime+=a->duration;}else e.animationTime=clampf(e.animationTime,0,a->duration);for(int t=0;t<a->trackCount;t++)if(a->tracks[t].bone<0){auto k=sampleKeys(a->tracks[t].keys,a->tracks[t].keyCount,e.animationTime);e.transform.x=e.source->transform.x+k.px;e.transform.y=e.source->transform.y+k.py;e.transform.z=e.source->transform.z+k.pz;e.transform.rx=e.source->transform.rx+k.rx;e.transform.ry=e.source->transform.ry+k.ry;e.transform.rz=e.source->transform.rz+k.rz;e.transform.sx=e.source->transform.sx*k.sx;e.transform.sy=e.source->transform.sy*k.sy;e.transform.sz=e.source->transform.sz*k.sz;break;}}}
void PS2SceneRuntime::dispatchCollision(PS2RuntimeEntity&a,PS2RuntimeEntity&b,float nx,float ny,float nz,int trigger){PS2CollisionEvent ea{&b,nx,ny,nz,trigger},eb{&a,-nx,-ny,-nz,trigger};if(a.source->script&&*a.source->script)ps2studio_component_collision(a.source->script,this,&a,&ea);if(b.source->script&&*b.source->script)ps2studio_component_collision(b.source->script,this,&b,&eb);}
void PS2SceneRuntime::updatePhysics(float dt){const float gravity=420.0f;for(int i=0;i<m_entityCount;i++){auto&e=m_entities[i];e.grounded=0;if(!e.active||!e.source->rigidBody||e.source->kinematic)continue;if(e.source->useGravity)e.vy+=gravity*e.source->gravityScale*dt;e.transform.x+=e.vx*dt;e.transform.y+=e.vy*dt;e.transform.z+=e.vz*dt;}
for(int i=0;i<m_entityCount;i++)for(int j=i+1;j<m_entityCount;j++){auto&a=m_entities[i];auto&b=m_entities[j];if(!a.active||!b.active||!a.source->colliderEnabled||!b.source->colliderEnabled)continue;if((a.source->collisionMask&b.source->collisionLayer)==0||(b.source->collisionMask&a.source->collisionLayer)==0)continue;if(!PS2Collision::aabb3(a,b))continue;m_stats.collisions++;float dx=b.transform.x-a.transform.x,dy=b.transform.y-a.transform.y,dz=b.transform.z-a.transform.z;float px=(a.source->colliderX+b.source->colliderX)*.5f-fabsf(dx),py=(a.source->colliderY+b.source->colliderY)*.5f-fabsf(dy),pz=(a.source->colliderZ+b.source->colliderZ)*.5f-fabsf(dz);float nx=0,ny=0,nz=0,pen=px;if(py<pen){pen=py;ny=dy>=0?-1:1;}else nx=dx>=0?-1:1;if(pz<pen){nx=ny=0;nz=dz>=0?-1:1;pen=pz;}int trig=a.source->trigger||b.source->trigger;if(!trig){bool ad=a.source->rigidBody&&!a.source->kinematic,bd=b.source->rigidBody&&!b.source->kinematic;float as=ad?(bd?.5f:1.0f):0,bs=bd?(ad?.5f:1.0f):0;a.transform.x+=nx*pen*as;a.transform.y+=ny*pen*as;a.transform.z+=nz*pen*as;b.transform.x-=nx*pen*bs;b.transform.y-=ny*pen*bs;b.transform.z-=nz*pen*bs;float rest=(a.source->restitution+b.source->restitution)*.5f;if(ad){if(nx)a.vx=-a.vx*rest;if(ny){a.vy=-a.vy*rest;if(ny<0)a.grounded=1;}if(nz)a.vz=-a.vz*rest;float fr=1-clampf(a.source->friction,0,1)*dt*10;if(!nx)a.vx*=fr;if(!nz)a.vz*=fr;}if(bd){if(nx)b.vx=-b.vx*rest;if(ny){b.vy=-b.vy*rest;if(ny>0)b.grounded=1;}if(nz)b.vz=-b.vz*rest;float fr=1-clampf(b.source->friction,0,1)*dt*10;if(!nx)b.vx*=fr;if(!nz)b.vz*=fr;}}dispatchCollision(a,b,nx,ny,nz,trig);}}
void PS2SceneRuntime::update(float dt){updateUI();for(int i=0;i<m_entityCount;i++){auto&e=m_entities[i];if(e.active&&e.source->script&&*e.source->script)ps2studio_component_update(e.source->script,this,&e,dt);}updateAnimations(dt);updatePhysics(dt);}

static void project(float x,float y,float z,float&sx,float&sy,int&sz){float d=z+6.0f;if(d<0.2f)d=0.2f;float f=300.0f/d;sx=320.0f+x*f;sy=224.0f-y*f;sz=(int)(0x10000+(d*1000));}
static u64 rgba(float r,float g,float b,float a){return GS_SETREG_RGBAQ((int)(clampf(r,0,1)*128),(int)(clampf(g,0,1)*128),(int)(clampf(b,0,1)*128),(int)(clampf(a,0,1)*128),0);}
void PS2SceneRuntime::drawSprite(const PS2RuntimeEntity&e){int i=-1;auto*a=findTexture(e.source->asset,&i);if(!a||i<0)return;auto&t=m_textures[i];float w=a->width*e.transform.sx,h=a->height*e.transform.sy;gsKit_prim_sprite_texture(m_gs,&t,e.transform.x,e.transform.y,0,0,e.transform.x+w,e.transform.y+h,a->width,a->height,2,rgba(e.source->tintR,e.source->tintG,e.source->tintB,e.source->alpha));m_stats.drawCalls++;}
void PS2SceneRuntime::drawMeshEE(const PS2RuntimeEntity&e){auto*skin=findSkinMesh(e.source->asset);if(skin){drawSkinnedMesh(e);return;}auto*m=findMesh(e.source->asset);if(!m)return;int ti=-1;auto*ta=findTexture(e.source->material,&ti);GSTEXTURE*tex=(ta&&ti>=0)?&m_textures[ti]:nullptr;float light=1.0f;if(!e.source->unlit){light=.25f;for(int q=0;q<m_entityCount;q++)if(m_entities[q].source->type==PS2_ENTITY_LIGHT){light=clampf(.2f+m_entities[q].source->lightIntensity*.8f,0,1);break;}}for(int k=0;k+2<m->indexCount;k+=3){float sxv[3],syv[3],uv[6];int z[3];float nz=0;for(int n=0;n<3;n++){int v=m->indices[k+n],vi=v*3,ui=v*2;float x=m->xyz[vi]*e.transform.sx+e.transform.x,y=m->xyz[vi+1]*e.transform.sy+e.transform.y,zz=m->xyz[vi+2]*e.transform.sz+e.transform.z;project(x,y,zz,sxv[n],syv[n],z[n]);uv[n*2]=m->uv?m->uv[ui]*(ta?ta->width:1):0;uv[n*2+1]=m->uv?m->uv[ui+1]*(ta?ta->height:1):0;if(m->normal)nz+=fabsf(m->normal[vi+2]);}float shade=e.source->unlit?1.0f:clampf(.2f+(nz/3.0f)*light,0.15f,1.0f);u64 col=rgba(e.source->tintR*shade,e.source->tintG*shade,e.source->tintB*shade,e.source->alpha);if(tex)gsKit_prim_triangle_texture_3d(m_gs,tex,sxv[0],syv[0],z[0],uv[0],uv[1],sxv[1],syv[1],z[1],uv[2],uv[3],sxv[2],syv[2],z[2],uv[4],uv[5],col);else gsKit_prim_triangle_3d(m_gs,sxv[0],syv[0],z[0],sxv[1],syv[1],z[1],sxv[2],syv[2],z[2],col);m_stats.drawCalls++;m_stats.triangles++;}}
void PS2SceneRuntime::drawSkinnedMesh(const PS2RuntimeEntity&e){auto*m=findSkinMesh(e.source->asset);if(!m||m->boneCount<=0||m->boneCount>64)return;M4 boneWorld[64],finalM[64];for(int i=0;i<m->boneCount;i++){M4 local=ident4();auto*a=findAnimation(e.source->animation);if(a){for(int t=0;t<a->trackCount;t++)if(a->tracks[t].bone==i){auto k=sampleKeys(a->tracks[t].keys,a->tracks[t].keyCount,e.animationTime);local=trs4(k.px,k.py,k.pz,k.rx,k.ry,k.rz,k.sx,k.sy,k.sz);break;}}int p=m->bones[i].parent;boneWorld[i]=(p>=0&&p<i)?mul4(boneWorld[p],local):local;finalM[i]=mul4(boneWorld[i],fromArray(m->bones[i].invBind));}int ti=-1;auto*ta=findTexture(e.source->material,&ti);GSTEXTURE*tex=(ta&&ti>=0)?&m_textures[ti]:nullptr;for(int k=0;k+2<m->indexCount;k+=3){float sxv[3],syv[3],uv[6];int z[3];for(int n=0;n<3;n++){const auto&v=m->vertices[m->indices[k+n]];float px=0,py=0,pz=0,ws=0;for(int w=0;w<4;w++){if(v.weights[w]<=0||v.bones[w]>=m->boneCount)continue;float tx,ty,tz;transformPoint(finalM[v.bones[w]],v.x,v.y,v.z,tx,ty,tz);px+=tx*v.weights[w];py+=ty*v.weights[w];pz+=tz*v.weights[w];ws+=v.weights[w];}if(ws<=0){px=v.x;py=v.y;pz=v.z;}px=px*e.transform.sx+e.transform.x;py=py*e.transform.sy+e.transform.y;pz=pz*e.transform.sz+e.transform.z;project(px,py,pz,sxv[n],syv[n],z[n]);uv[n*2]=v.u*(ta?ta->width:1);uv[n*2+1]=v.v*(ta?ta->height:1);}u64 col=rgba(e.source->tintR,e.source->tintG,e.source->tintB,e.source->alpha);if(tex)gsKit_prim_triangle_texture_3d(m_gs,tex,sxv[0],syv[0],z[0],uv[0],uv[1],sxv[1],syv[1],z[1],uv[2],uv[3],sxv[2],syv[2],z[2],uv[4],uv[5],col);else gsKit_prim_triangle_3d(m_gs,sxv[0],syv[0],z[0],sxv[1],syv[1],z[1],sxv[2],syv[2],z[2],col);m_stats.drawCalls++;m_stats.triangles++;}}
void PS2SceneRuntime::drawMeshVU1(const PS2RuntimeEntity&e){/* VU1 build and microprogram are real; dispatch remains opt-in until DMA/VIF synchronization is verified on hardware. */drawMeshEE(e);}
static const char*glyph(char c){if(c>='a'&&c<='z')c=(char)(c-'a'+'A');switch(c){case'A':return"010101111101101";case'B':return"110101110101110";case'C':return"011100100100011";case'D':return"110101101101110";case'E':return"111100110100111";case'F':return"111100110100100";case'G':return"011100101101011";case'H':return"101101111101101";case'I':return"111010010010111";case'J':return"001001001101010";case'K':return"101101110101101";case'L':return"100100100100111";case'M':return"101111111101101";case'N':return"101111111111101";case'O':return"010101101101010";case'P':return"110101110100100";case'Q':return"010101101111011";case'R':return"110101110101101";case'S':return"011100010001110";case'T':return"111010010010010";case'U':return"101101101101111";case'V':return"101101101101010";case'W':return"101101111111101";case'X':return"101101010101101";case'Y':return"101101010010010";case'Z':return"111001010100111";case'0':return"111101101101111";case'1':return"010110010010111";case'2':return"110001111100111";case'3':return"110001111001110";case'4':return"101101111001001";case'5':return"111100110001110";case'6':return"011100110101010";case'7':return"111001010010010";case'8':return"111101111101111";case'9':return"111101111001110";case'-':return"000000111000000";case'.':return"000000000000010";case':':return"000010000010000";case'/':return"001001010100100";case'_':return"000000000000111";default:return"000000000000000";}}
void PS2SceneRuntime::drawText(const PS2RuntimeEntity&e){if(!e.source->uiText)return;float px=2.0f*(e.transform.sx<=0?1.0f:e.transform.sx),x=e.transform.x,y=e.transform.y;u64 col=rgba(e.source->uiR,e.source->uiG,e.source->uiB,e.source->alpha);for(const char*p=e.source->uiText;*p;++p){if(*p=='\n'){y+=7*px;x=e.transform.x;continue;}if(*p==' '){x+=4*px;continue;}const char*g=glyph(*p);for(int i=0;i<15;i++)if(g[i]=='1'){int gx=i%3,gy=i/3;gsKit_prim_sprite(m_gs,x+gx*px,y+gy*px,4,x+(gx+1)*px,y+(gy+1)*px,4,col);}x+=4*px;}m_stats.drawCalls++;}
void PS2SceneRuntime::drawUIBox(const PS2RuntimeEntity&e,bool selected){float r=e.source->uiR,g=e.source->uiG,b=e.source->uiB;if(selected){r=clampf(r+.25f,0,1);g=clampf(g+.25f,0,1);b=clampf(b+.25f,0,1);}gsKit_prim_sprite(m_gs,e.transform.x,e.transform.y,3,e.transform.x+e.source->uiW,e.transform.y+e.source->uiH,3,rgba(r,g,b,e.source->alpha));m_stats.drawCalls++;if(e.source->uiText)drawText(e);}
void PS2SceneRuntime::drawParticles(const PS2RuntimeEntity&e){if(!e.source->particleEmitter)return;int count=e.source->particleCount>64?64:e.source->particleCount;u64 c=rgba(1,.7f,.2f,1);for(int i=0;i<count;i++){float a=(i+m_stats.frame*.5f)*.67f,d=(float)(i%13)*2.0f;float x=e.transform.x+cosf(a)*d,y=e.transform.y+sinf(a)*d;gsKit_prim_point(m_gs,x,y,3,c);}m_stats.drawCalls++;}
void PS2SceneRuntime::drawEntity(const PS2RuntimeEntity&e){if(!e.active)return;switch(e.source->type){case PS2_ENTITY_SPRITE:case PS2_ENTITY_IMAGE:drawSprite(e);break;case PS2_ENTITY_MESH:(m_backend==PS2_RENDERER_VU1_EXPERIMENTAL?drawMeshVU1(e):drawMeshEE(e));break;case PS2_ENTITY_TEXT:drawText(e);break;case PS2_ENTITY_PANEL:case PS2_ENTITY_BUTTON:drawUIBox(e,m_uiSelected>=0&&(&e==&m_entities[m_uiSelected]));break;default:break;}drawParticles(e);m_stats.entities++;}
void PS2SceneRuntime::drawCurrentScene(){for(int i=0;i<m_entityCount;i++)drawEntity(m_entities[i]);}
void PS2SceneRuntime::endFrame(){gsKit_queue_exec(m_gs);gsKit_sync_flip(m_gs);m_stats.frame++;if((m_stats.frame%120)==0)printf("[PS2StudioProfiler] frame=%u entities=%u drawCalls=%u triangles=%u collisions=%u audio=%u physicsPairs=%u skinVertices=%u vifPackets=%u vuDispatches=%u\n",m_stats.frame,m_stats.entities,m_stats.drawCalls,m_stats.triangles,m_stats.collisions,m_stats.audioVoices,m_stats.physicsPairs,m_stats.skinVertices,m_stats.vifPackets,m_stats.vuDispatches);}
)PS2";}

QString ps2MainSource(const QString&type){Q_UNUSED(type);return R"PS2(#include <tamtypes.h>
#include <kernel.h>
#include <gsKit.h>
#include <dmaKit.h>
#include "PS2SceneRuntime.h"
int main(){GSGLOBAL*gs=gsKit_init_global();gs->PSM=GS_PSM_CT24;gs->PSMZ=GS_PSMZ_16S;gs->PrimAlphaEnable=GS_SETTING_ON;dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF,D_CTRL_STS_UNSPEC,D_CTRL_STD_OFF,D_CTRL_RCYC_8,1<<DMA_CHANNEL_GIF);dmaKit_chan_init(DMA_CHANNEL_GIF);gsKit_init_screen(gs);PS2SceneRuntime rt(gs);rt.initialize();
#ifdef PS2STUDIO_VU1
rt.setRendererBackend(PS2_RENDERER_VU1_EXPERIMENTAL);
#endif
while(true){rt.beginFrame(GS_SETREG_RGBAQ(12,16,24,0x80,0));rt.update(1.0f/60.0f);rt.drawCurrentScene();rt.endFrame();}return 0;}
)PS2";}

QString ps2VuProgram(){return R"VU(.syntax new
.name PS2StudioTransform
--enter
--endenter
    lq.xyzw vf01, 0(vi00)     | nop
    lq.xyzw vf02, 1(vi00)     | nop
    lq.xyzw vf03, 2(vi00)     | nop
    lq.xyzw vf04, 3(vi00)     | nop
--exit
--endexit
)VU";}

QString ps2Makefile(const QString&name){return QString(R"MK(EE_BIN = %1.elf
SCRIPT_CPP := $(wildcard scripts/*.cpp)
SCRIPT_OBJS := $(SCRIPT_CPP:.cpp=.o)
EE_OBJS = src/main.o runtime/PS2SceneRuntime.o generated/PS2Assets.o generated/PS2SceneData.o generated/PS2ComponentRegistry.o $(SCRIPT_OBJS)
EE_INCS += -Iruntime -Iinclude -Igenerated -Iscripts -I$(GSKIT)/ee/gs/include -I$(GSKIT)/ee/dma/include
EE_LDFLAGS += -L$(GSKIT)/lib
EE_LIBS = -lgskit -ldmakit -lgskit_toolkit -lpad -laudsrv -lpatches -lkernel -lm
PROFILE ?= Debug
ifeq ($(PROFILE),Release)
  EE_CFLAGS += -O2 -DNDEBUG
  EE_CXXFLAGS += -O2 -DNDEBUG
else ifeq ($(PROFILE),Optimized)
  EE_CFLAGS += -O2 -g
  EE_CXXFLAGS += -O2 -g
else
  EE_CFLAGS += -O0 -g -DDEBUG
  EE_CXXFLAGS += -O0 -g -DDEBUG
endif
ifeq ($(VU1),1)
  EE_CFLAGS += -DPS2STUDIO_VU1
  EE_CXXFLAGS += -DPS2STUDIO_VU1
  VU1_OBJ = runtime/vu1_transform.o
  EE_OBJS += $(VU1_OBJ)
endif
runtime/%%.o: runtime/%%.vsm
	dvp-as $< -o $@
all: $(EE_BIN)
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
)MK").arg(name);}

QString ps2ComponentExample(const QString&className){QString c=className;c.replace(QRegularExpression("[^A-Za-z0-9_]"),"_");if(c.isEmpty())c="GameComponent";return QString(R"CPP(#include "PS2SceneRuntime.h"
#include <string.h>

// PS2STUDIO_COMPONENT: %1
extern "C" void %1_Start(PS2SceneRuntime* runtime,PS2RuntimeEntity* self){
    (void)runtime;
    // Called once when the scene loads.
    self->vx = 0.0f;
}

extern "C" void %1_Update(PS2SceneRuntime* runtime,PS2RuntimeEntity* self,float dt){
    (void)dt;
    // Example player movement.
    if(runtime->input.down(PAD_LEFT)) self->transform.x -= 2.5f;
    if(runtime->input.down(PAD_RIGHT)) self->transform.x += 2.5f;
    if(runtime->input.down(PAD_UP)) self->transform.y -= 2.5f;
    if(runtime->input.down(PAD_DOWN)) self->transform.y += 2.5f;
}

extern "C" void %1_OnCollision(PS2SceneRuntime* runtime,PS2RuntimeEntity* self,const PS2CollisionEvent* event){
    (void)runtime;(void)self;(void)event;
}

extern "C" void %1_OnAction(PS2SceneRuntime* runtime,PS2RuntimeEntity* self,const char* action){
    (void)self;
    if(action && strcmp(action,"PLAY_SFX")==0) runtime->playSound("sample.wav");
}
)CPP").arg(c);}
