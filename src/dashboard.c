#include "dashboard.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

/* A shared canvas transform keeps drawing and hit testing aligned on resize. */
#define CANVAS_W 1440.0f
#define CANVAS_H 960.0f
static const Color bg={13,16,19,255}, sidebar={17,20,23,255};
static const Color surface={22,26,30,255}, raised={30,35,39,255}, edge={43,49,53,255};
static const Color text={238,242,235,255}, muted={150,161,163,255}, subtle={100,115,119,255};
static const Color lime={201,242,112,255};
static const Color palette[]={
    {201,242,112,255},{130,181,242,255},{185,153,239,255},{245,188,121,255},
    {111,207,183,255},{235,145,160,255},{138,211,234,255},{216,205,151,255},
    {170,184,207,255},{223,169,217,255}
};
static const char *regionNames[]={"All regions","North","South","East","West"};
static const char *months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static Font regularFont,boldFont;
static int page,tablePage,sortMode;
static bool lineChart,searchFocus,exportRequested,snapshotRequested,snapshotRendering;
static char query[48],notice[160];
static double noticeUntil;
static float scale=1,offsetX,offsetY;
static int matched[MAX_SALES],matchCount,visibleRows=3;
static RegionFilter lastRegion;

static Rectangle rect(float x,float y,float w,float h) { return (Rectangle){x,y,w,h}; }
static Vector2 point(float x,float y) { return (Vector2){x,y}; }
static void transform(void)
{
    scale=fminf((float)GetScreenWidth()/CANVAS_W,(float)GetScreenHeight()/CANVAS_H);
    offsetX=((float)GetScreenWidth()-CANVAS_W*scale)/2;
    offsetY=((float)GetScreenHeight()-CANVAS_H*scale)/2;
}
static Vector2 mouse(void)
{
    Vector2 p=GetMousePosition(); return point((p.x-offsetX)/scale,(p.y-offsetY)/scale);
}
static bool hover(Rectangle r) { return !snapshotRendering&&CheckCollisionPointRec(mouse(),r); }
static bool click(Rectangle r) { return hover(r)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT); }
static Font font(float size) { return size>=20?boldFont:regularFont; }
static float measure(const char *s,float size) { return MeasureTextEx(font(size),s,size,0).x; }
static void label(const char *s,float x,float y,float size,Color c) { DrawTextEx(font(size),s,point(x,y),size,0,c); }
static void rightLabel(const char *s,float x,float y,float size,Color c) { label(s,x-measure(s,size),y,size,c); }
static void centered(const char *s,Rectangle r,float size,Color c)
{
    label(s,r.x+(r.width-measure(s,size))/2,r.y+(r.height-size)/2,size,c);
}
static void clipped(const char *s,float x,float y,float width,float size,Color c)
{
    char tmp[96]; snprintf(tmp,sizeof tmp,"%s",s); size_t n=strlen(tmp);
    if(measure(tmp,size)>width) {
        while(n>0&&measure(tmp,size)>width-12) tmp[--n]='\0';
        if(n+3<sizeof tmp) { tmp[n++]='.'; tmp[n++]='.'; tmp[n++]='.'; tmp[n]='\0'; }
    }
    label(tmp,x,y,size,c);
}
static void rounded(Rectangle r,Color c) { DrawRectangleRounded(r,0.14f,10,c); }
static void panel(Rectangle r)
{
    rounded(rect(r.x,r.y+3,r.width,r.height),(Color){0,0,0,35});
    rounded(r,surface); DrawRectangleRoundedLinesEx(r,0.14f,10,1,edge);
}
static void line(float x1,float y1,float x2,float y2,Color c) { DrawLineEx(point(x1,y1),point(x2,y2),1,c); }
static void icon(int type,float x,float y,Color c)
{
    if(type==0) {
        for(int i=0;i<4;i++) rounded(rect(x+(i%2)*9,y+(i/2)*9,6,6),c);
    } else if(type==1) {
        for(int i=0;i<3;i++) { line(x,y+3+i*6,x+17,y+3+i*6,c); DrawCircleV(point(x-4,y+3+i*6),1,c); }
    } else if(type==2) {
        DrawCircleLinesV(point(x+7,y+7),6,c); DrawLineEx(point(x+12,y+12),point(x+17,y+17),2,c);
    } else if(type==3) {
        DrawLineEx(point(x+8,y),point(x+8,y+12),1.6f,c);
        DrawLineEx(point(x+3,y+7),point(x+8,y+12),1.6f,c);
        DrawLineEx(point(x+8,y+12),point(x+13,y+7),1.6f,c);
        line(x,y+13,x,y+17,c); line(x,y+17,x+16,y+17,c); line(x+16,y+17,x+16,y+13,c);
    } else {
        DrawCircleLinesV(point(x+8,y+8),7,c); line(x+8,y+3,x+8,y+9,c); line(x+8,y+9,x+12,y+9,c);
    }
}
static void money(char *out,size_t size,double value)
{
    char raw[64]; snprintf(raw,sizeof raw,"%.2f",fabs(value));
    const char *dot=strchr(raw,'.'); int whole=dot?(int)(dot-raw):(int)strlen(raw); size_t pos=0;
    if(value<0&&pos+1<size) out[pos++]='-';
    if(pos+1<size) out[pos++]='$';
    for(int i=0;i<whole&&pos+1<size;i++) {
        out[pos++]=raw[i];
        if((whole-i-1)%3==0&&i+1<whole&&pos+1<size) out[pos++]=',';
    }
    if(dot) for(int i=0;dot[i]&&pos+1<size;i++) out[pos++]=dot[i];
    out[pos]='\0';
}
static Rectangle filterBounds(void) { return rect(244,126,570,38); }
static Rectangle filterButton(Rectangle r,int i) { return rect(r.x+i*114,r.y,108,r.height); }
static Rectangle tableBounds(void) { return page==1?rect(244,188,1168,716):rect(244,666,748,238); }
static Rectangle searchBounds(void) { Rectangle r=tableBounds(); return rect(r.x+r.width-256,r.y+17,232,32); }

/* Original B monogram: two rounded data blocks and a shared vertical spine.
   The same geometry is used for the sidebar, window icon, and SVG asset. */
static void drawBrandMark(Rectangle bounds)
{
    float s=bounds.width/64.0f;
    float x=bounds.x,y=bounds.y;
    DrawRectangleRounded(bounds,0.5f,16,lime);
    DrawRectangleRounded(rect(x+14*s,y+12*s,30*s,21*s),0.85f,12,bg);
    DrawRectangleRounded(rect(x+14*s,y+29*s,35*s,23*s),0.85f,12,bg);
    DrawRectangleRec(rect(x+13*s,y+12*s,10*s,40*s),bg);
    DrawRectangleRounded(rect(x+23*s,y+19*s,12*s,7*s),0.85f,8,lime);
    DrawRectangleRounded(rect(x+23*s,y+36*s,17*s,8*s),0.85f,8,lime);
}

void initDashboard(void)
{
    regularFont=LoadFontEx("assets/fonts/Lato-Regular.ttf",48,NULL,0);
    boldFont=LoadFontEx("assets/fonts/Lato-Bold.ttf",64,NULL,0);
    SetTextureFilter(regularFont.texture,TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(boldFont.texture,TEXTURE_FILTER_BILINEAR); transform();
    RenderTexture2D mark=LoadRenderTexture(64,64);
    if(mark.id!=0) {
        BeginTextureMode(mark);
        ClearBackground(BLANK);
        drawBrandMark(rect(0,0,64,64));
        EndTextureMode();
        Image image=LoadImageFromTexture(mark.texture);
        ImageFlipVertical(&image);
        SetWindowIcon(image);
        UnloadImage(image);
        UnloadRenderTexture(mark);
    }
}
void closeDashboard(void) { UnloadFont(regularFont); UnloadFont(boldFont); }
bool takeDashboardExportRequest(void) { bool r=exportRequested; exportRequested=false; return r; }
bool takeDashboardSnapshotRequest(void) { bool r=snapshotRequested; snapshotRequested=false; return r; }
void setDashboardNotice(const char *message) { snprintf(notice,sizeof notice,"%s",message); noticeUntil=GetTime()+5; }

void updateDashboardInput(RegionFilter *selectedRegion)
{
    transform(); SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    if(click(rect(16,147,192,44))) { page=0; tablePage=0; searchFocus=false; }
    if(click(rect(16,199,192,44))) { page=1; tablePage=0; searchFocus=false; }
    for(int i=0;i<5;i++) {
        if(click(filterButton(filterBounds(),i))) *selectedRegion=(RegionFilter)i;
        if(!searchFocus&&IsKeyPressed(KEY_ONE+i)) *selectedRegion=(RegionFilter)i;
    }
    if(page==0) {
        if(click(rect(814,361,64,28))) lineChart=false;
        if(click(rect(879,361,64,28))) lineChart=true;
        for(int i=0;i<4;i++) if(click(rect(1030,730+i*38.0f,364,34))) *selectedRegion=(RegionFilter)(i+1);
    }
    if(*selectedRegion!=lastRegion) { tablePage=0; lastRegion=*selectedRegion; }
    if(click(rect(1240,49,172,40))) exportRequested=true;
    if(click(rect(1050,49,174,40))) snapshotRequested=true;
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) searchFocus=hover(searchBounds());
    int ch=GetCharPressed();
    while(ch>0) {
        size_t n=strlen(query);
        if(searchFocus&&ch>=32&&ch<=126&&n+1<sizeof query) { query[n]=(char)ch; query[n+1]='\0'; tablePage=0; }
        if(!searchFocus&&ch>='1'&&ch<='5') { *selectedRegion=(RegionFilter)(ch-'1'); tablePage=0; }
        ch=GetCharPressed();
    }
    if(searchFocus&&(IsKeyPressed(KEY_BACKSPACE)||IsKeyPressedRepeat(KEY_BACKSPACE))) {
        size_t n=strlen(query); if(n) query[n-1]='\0'; tablePage=0;
    }
    if(searchFocus&&IsKeyPressed(KEY_DELETE)) { query[0]='\0'; tablePage=0; }
    if(searchFocus&&IsKeyPressed(KEY_ESCAPE)) { searchFocus=false; query[0]='\0'; tablePage=0; }
    if(searchFocus&&(IsKeyDown(KEY_LEFT_CONTROL)||IsKeyDown(KEY_RIGHT_CONTROL))&&IsKeyPressed(KEY_V)) {
        const char *paste=GetClipboardText();
        size_t n=strlen(query);
        if(paste) for(size_t i=0;paste[i]&&n+1<sizeof query;i++)
            if((unsigned char)paste[i]>=32&&(unsigned char)paste[i]<=126) query[n++]=paste[i];
        query[n]='\0'; tablePage=0;
    }
    Rectangle t=tableBounds();
    if(click(rect(t.x+140,t.y+65,120,28))) { sortMode=sortMode==1?0:1; tablePage=0; }
    if(click(rect(t.x+t.width-148,t.y+65,120,28))) { sortMode=sortMode==2?3:2; tablePage=0; }
    if(click(rect(t.x+t.width-84,t.y+t.height-31,28,24))&&tablePage>0) tablePage--;
    if(click(rect(t.x+t.width-48,t.y+t.height-31,28,24))&&(tablePage+1)*visibleRows<matchCount) tablePage++;
}

static void drawSidebar(const DashboardViewData *data)
{
    DrawRectangle(0,0,224,960,sidebar); line(224,0,224,960,edge);
    drawBrandMark(rect(24,33,36,36));
    label("BI",70,41,21,lime);
    label("Dashboard",70+measure("BI",21),41,21,text);
    label("BUSINESS INTELLIGENCE",25,82,10,subtle);
    label("WORKSPACE",26,120,11,subtle);
    const char *titles[]={"Overview","Transactions"};
    for(int i=0;i<2;i++) {
        Rectangle r=rect(16,147+i*52.0f,192,44);
        if(page==i) rounded(r,(Color){40,48,36,255}); else if(hover(r)) rounded(r,raised);
        icon(i,31,r.y+14,page==i?lime:muted); label(titles[i],62,r.y+13,16,page==i?lime:muted);
        if(page==i) DrawCircleV(point(190,r.y+22),3,lime);
        if(hover(r)) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }
    line(24,283,200,283,edge);
    label("PROFITABILITY",26,311,11,subtle);
    Vector2 center=point(112,414);
    DrawRing(center,58,65,0,360,100,raised);
    float margin=(float)fmin(fmax(data->profitMargin,0),100);
    if(margin>0) DrawRing(center,58,65,-90,-90+margin*3.6f,100,lime);
    char percentage[32]; snprintf(percentage,sizeof percentage,"%.1f%%",data->profitMargin);
    centered(percentage,rect(45,393,134,35),28,text);
    centered("NET MARGIN",rect(45,432,134,16),10,muted);
    char amount[64];
    label("Total cost",26,515,12,muted); money(amount,sizeof amount,data->stats.totalCost);
    label(amount,26,535,20,text);
    label("Net profit",26,582,12,muted); money(amount,sizeof amount,data->stats.totalProfit);
    label(amount,26,602,20,lime);
    rounded(rect(16,759,192,115),surface);
    DrawCircleV(point(34,782),3,lime); label("CSV CONNECTED",45,775,11,lime);
    label("sales.csv",30,800,18,text); label("Local data. Real insights.",30,832,12,muted);
    line(24,899,200,899,edge); DrawCircleV(point(40,930),15,raised);
    centered("EA",rect(25,915,30,30),11,lime);
    label("Ege & Umut",65,916,14,text); label("Advanced Programming",65,936,10,subtle);
}

void drawFilterButtons(Rectangle bounds,RegionFilter *selectedRegion)
{
    for(int i=0;i<5;i++) {
        Rectangle r=filterButton(bounds,i); bool active=*selectedRegion==(RegionFilter)i;
        rounded(r,active?lime:(hover(r)?raised:surface)); centered(regionNames[i],r,14,active?bg:muted);
        if(hover(r)) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }
}
void drawKpiCard(Rectangle r,const char *title,const char *value)
{
    panel(r); label(title,r.x+22,r.y+20,14,muted);
    float size=32;
    while(size>18&&measure(value,size)>r.width-44) size--;
    label(value,r.x+22,r.y+47,size,text);
}
static void drawKpis(const DashboardViewData *data)
{
    const char *names[]={"Total revenue","Net profit","Total orders","Average order value"};
    char values[4][64],notes[4][64];
    money(values[0],sizeof values[0],data->stats.totalRevenue);
    money(values[1],sizeof values[1],data->stats.totalProfit);
    snprintf(values[2],sizeof values[2],"%d",data->stats.totalOrders);
    money(values[3],sizeof values[3],data->stats.averageOrderValue);
    snprintf(notes[0],sizeof notes[0],"Across %d active months",data->activeMonths);
    snprintf(notes[1],sizeof notes[1],"%.1f%% profit margin",data->profitMargin);
    snprintf(notes[2],sizeof notes[2],"%d records in dataset",data->loadedCount);
    snprintf(notes[3],sizeof notes[3],"Revenue per transaction");
    for(int i=0;i<4;i++) {
        Rectangle r=rect(244+i*296.0f,188,280,132); drawKpiCard(r,names[i],values[i]);
        if(i==0) { DrawRectangleRoundedLinesEx(r,0.14f,10,1,(Color){91,111,63,255}); rounded(rect(r.x+20,r.y+106,7,3),lime); }
        else DrawCircleV(point(r.x+24,r.y+108),2,i==1?lime:palette[i]);
        label(notes[i],r.x+34,r.y+99,12,i==1?lime:subtle); icon(i==2?1:4,r.x+239,r.y+20,subtle);
    }
}

void drawMonthlyChart(Rectangle r,const double values[MONTH_COUNT])
{
    panel(r); label("Revenue overview",r.x+24,r.y+22,20,text);
    label("Monthly performance across the selected region",r.x+24,r.y+51,12,muted);
    rounded(rect(r.x+r.width-158,r.y+18,134,34),bg);
    Rectangle bars=rect(r.x+r.width-154,r.y+21,64,28), lines=rect(r.x+r.width-89,r.y+21,64,28);
    rounded(lineChart?lines:bars,raised);
    centered("Bars",bars,12,!lineChart?text:subtle); centered("Line",lines,12,lineChart?text:subtle);
    if(hover(bars)||hover(lines)) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    double maximum=1; int best=-1;
    for(int i=0;i<MONTH_COUNT;i++) if(values[i]>maximum) { maximum=values[i]; best=i; }
    maximum*=1.22;
    float left=r.x+70,top=r.y+100,w=r.width-96,h=r.height-143;
    for(int i=0;i<4;i++) {
        float y=top+h*(float)i/3; char tick[32]; double v=maximum*(3-i)/3;
        if(v>=1000) snprintf(tick,sizeof tick,"$%.1fk",v/1000); else snprintf(tick,sizeof tick,"$%.0f",v);
        rightLabel(tick,left-16,y-7,11,subtle); line(left,y,left+w,y,edge);
    }
    Vector2 pts[MONTH_COUNT]; float slot=w/MONTH_COUNT; int active=-1;
    for(int i=0;i<MONTH_COUNT;i++) {
        float x=left+slot*((float)i+0.5f),bh=(float)(fmax(values[i],0)/maximum)*h;
        pts[i]=point(x,top+h-bh); bool over=hover(rect(left+slot*i,top,slot,h+24)); if(over) active=i;
        if(!lineChart) {
            rounded(rect(x-slot*0.27f,top,slot*0.54f,h),(Color){29,35,32,255});
            if(bh>0) rounded(rect(x-slot*0.27f,top+h-bh,slot*0.54f,bh),over?text:(i==best?lime:(Color){126,158,83,255}));
        }
        centered(months[i],rect(left+slot*i,top+h+15,slot,16),12,over?lime:muted);
    }
    if(lineChart) {
        for(int i=0;i<MONTH_COUNT-1;i++) {
            DrawTriangle(pts[i],point(pts[i].x,top+h),pts[i+1],Fade(lime,0.07f));
            DrawTriangle(pts[i+1],point(pts[i].x,top+h),point(pts[i+1].x,top+h),Fade(lime,0.07f));
            DrawLineEx(pts[i],pts[i+1],2.5f,lime);
        }
        for(int i=0;i<MONTH_COUNT;i++) { DrawCircleV(pts[i],5,surface); DrawCircleV(pts[i],3,lime); }
    }
    if(active>=0) {
        Vector2 p=pts[active]; line(p.x,top,p.x,top+h,Fade(lime,0.45f));
        char amount[64],tooltip[96]; money(amount,sizeof amount,values[active]);
        snprintf(tooltip,sizeof tooltip,"%s  %s",months[active],amount);
        float tw=measure(tooltip,13)+24,tx=fminf(fmaxf(p.x-tw/2,r.x+15),r.x+r.width-tw-15);
        rounded(rect(tx,top-31,tw,26),lime); label(tooltip,tx+12,top-25,13,bg);
    }
}

void drawCategoryChart(Rectangle r,const CategoryRevenue categories[],int categoryCount)
{
    panel(r); label("Revenue mix",r.x+24,r.y+22,20,text); label("Contribution by product category",r.x+24,r.y+51,12,muted);
    if(categoryCount<=0) { label("No category data for this region",r.x+24,r.y+120,15,muted); return; }
    if(categoryCount>MAX_CATEGORIES) categoryCount=MAX_CATEGORIES;
    float row=fminf(51,(r.height-114)/(float)categoryCount);
    for(int i=0;i<categoryCount;i++) {
        float y=r.y+103+i*row; char amount[64]; money(amount,sizeof amount,categories[i].revenue);
        DrawCircleV(point(r.x+27,y+7),3,palette[i]);
        clipped(categories[i].name,r.x+40,y,r.width-174,14,text); rightLabel(amount,r.x+r.width-24,y,14,text);
    }
}
static void drawMix(const DashboardViewData *data,Rectangle r)
{
    drawCategoryChart(r,data->categories,data->categoryCount); if(data->categoryCount<=0) return;
    float x=r.x+24; rounded(rect(x,r.y+78,r.width-48,7),raised);
    for(int i=0;i<data->categoryCount;i++) {
        float w=(r.width-48)*(float)(data->categoryShare[i]/100);
        if(w>0) DrawRectangleRec(rect(x,r.y+78,w,7),palette[i]); x+=w;
    }
    float row=fminf(51,(r.height-114)/(float)data->categoryCount);
    for(int i=0;i<data->categoryCount;i++) {
        float y=r.y+103+i*row;
        if(row>=36) { char share[32]; snprintf(share,sizeof share,"%.1f%% of revenue",data->categoryShare[i]); label(share,r.x+40,y+20,11,subtle); }
        if(hover(rect(r.x+20,y-4,r.width-40,row))) DrawRectangleRoundedLinesEx(rect(r.x+15,y-6,r.width-30,row),0.2f,6,1,palette[i]);
    }
}

static bool contains(const char *hay,const char *needle)
{
    if(!needle[0]) return true;
    for(;*hay;hay++) {
        const char *a=hay,*b=needle;
        while(*a&&*b&&tolower((unsigned char)*a)==tolower((unsigned char)*b)) { a++; b++; }
        if(!*b) return true;
    }
    return false;
}
/* Sorting and searching only affect presentation, never business metrics. */
static void prepareRows(const DashboardViewData *data)
{
    static unsigned int lastRevision;
    static int lastSort=-1;
    static char lastQuery[sizeof query];
    if(lastRevision==data->revision&&lastSort==sortMode&&strcmp(lastQuery,query)==0) return;
    lastRevision=data->revision; lastSort=sortMode;
    snprintf(lastQuery,sizeof lastQuery,"%s",query);
    matchCount=0;
    for(int i=0;i<data->saleCount&&i<MAX_SALES;i++) {
        const Sale *s=&data->sales[i]; char id[24]; snprintf(id,sizeof id,"%d",s->id);
        if(contains(s->category,query)||contains(s->region,query)||contains(s->date,query)||contains(id,query)) matched[matchCount++]=i;
    }
    for(int i=1;i<matchCount;i++) {
        int key=matched[i],j=i-1;
        while(j>=0) {
            const Sale *a=&data->sales[matched[j]],*b=&data->sales[key];
            bool move=sortMode==2?a->revenue<b->revenue:sortMode==3?a->revenue>b->revenue:
                      sortMode==1?strcmp(a->date,b->date)>0:strcmp(a->date,b->date)<0;
            if(!move) break; matched[j+1]=matched[j]; j--;
        }
        matched[j+1]=key;
    }
}
static void drawTransactions(const DashboardViewData *data)
{
    Rectangle r=tableBounds(); panel(r); label(page==1?"Transaction explorer":"Recent transactions",r.x+22,r.y+21,20,text);
    Rectangle search=searchBounds(); rounded(search,bg);
    if(searchFocus) DrawRectangleRoundedLinesEx(search,0.2f,8,1,lime);
    icon(2,search.x+10,search.y+8,searchFocus?lime:subtle);
    clipped(query[0]?query:"Search records...",search.x+36,search.y+9,search.width-44,12,query[0]?text:subtle);
    if(searchFocus&&((int)(GetTime()*2)%2)==0) line(search.x+36+measure(query,12),search.y+8,search.x+36+measure(query,12),search.y+23,lime);
    if(hover(search)) SetMouseCursor(MOUSE_CURSOR_IBEAM);
    float dateX=r.x+140,categoryX=r.x+285,regionX=r.x+r.width-245,amountX=r.x+r.width-24;
    line(r.x+1,r.y+64,r.x+r.width-1,r.y+64,edge);
    label("ORDER",r.x+22,r.y+75,10,subtle); label(sortMode==1?"DATE  ^":"DATE  v",dateX,r.y+75,10,muted);
    label("CATEGORY",categoryX,r.y+75,10,subtle); label("REGION",regionX,r.y+75,10,subtle);
    rightLabel(sortMode==3?"REVENUE  ^":"REVENUE  v",amountX,r.y+75,10,muted);
    prepareRows(data); visibleRows=(int)((r.height-130)/35); if(visibleRows<1) visibleRows=1;
    int maxPage=matchCount>0?(matchCount-1)/visibleRows:0; if(tablePage>maxPage) tablePage=maxPage;
    for(int row=0;row<visibleRows;row++) {
        int index=tablePage*visibleRows+row; if(index>=matchCount) break;
        const Sale *s=&data->sales[matched[index]]; float y=r.y+102+row*35.0f;
        if(hover(rect(r.x+8,y-5,r.width-16,33))) rounded(rect(r.x+8,y-5,r.width-16,33),raised);
        char id[32],amount[64]; snprintf(id,sizeof id,"#%04d",s->id); money(amount,sizeof amount,s->revenue);
        label(id,r.x+22,y,13,text); label(s->date,dateX,y,13,muted); clipped(s->category,categoryX,y,regionX-categoryX-15,13,text);
        rounded(rect(regionX-8,y-3,69,22),raised); label(s->region,regionX,y+1,11,muted); rightLabel(amount,amountX,y,13,text);
        line(r.x+22,y+26,r.x+r.width-22,y+26,Fade(edge,0.5f));
    }
    if(matchCount==0) label("No matching records. Try a different search or region.",r.x+22,r.y+115,14,muted);
    char summary[80]; snprintf(summary,sizeof summary,"%d-%d of %d records",matchCount?tablePage*visibleRows+1:0,
        (tablePage+1)*visibleRows<matchCount?(tablePage+1)*visibleRows:matchCount,matchCount);
    label(summary,r.x+22,r.y+r.height-26,11,subtle);
    Rectangle prev=rect(r.x+r.width-84,r.y+r.height-31,28,24),next=rect(r.x+r.width-48,r.y+r.height-31,28,24);
    rounded(prev,hover(prev)?raised:bg); rounded(next,hover(next)?raised:bg);
    centered("<",prev,14,tablePage>0?text:subtle); centered(">",next,14,tablePage<maxPage?text:subtle);
    if(hover(prev)||hover(next)||hover(rect(dateX,r.y+65,120,28))||hover(rect(amountX-124,r.y+65,120,28))) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
}
static void drawRegions(const DashboardViewData *data,RegionFilter selected)
{
    Rectangle r=rect(1012,666,400,238); panel(r); label("Regional performance",r.x+24,r.y+22,20,text);
    label("Share of total dataset revenue",r.x+24,r.y+49,12,muted);
    for(int i=0;i<4;i++) {
        float y=r.y+69+i*38.0f; bool active=selected==(RegionFilter)(i+1); Rectangle hit=rect(r.x+18,y-5,r.width-36,34);
        if(active||hover(hit)) rounded(hit,raised);
        label(regionNames[i+1],r.x+24,y+3,13,active?lime:text); rounded(rect(r.x+102,y+9,114,4),bg);
        rounded(rect(r.x+102,y+9,114*(float)(data->regionShare[i+1]/100),4),active?lime:palette[i]);
        char amount[64]; money(amount,sizeof amount,data->regionStats[i+1].totalRevenue); rightLabel(amount,r.x+r.width-24,y+3,13,muted);
        if(hover(hit)) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }
}
void drawDashboard(const DashboardViewData *data,RegionFilter *selectedRegion)
{
    transform(); ClearBackground(bg); Camera2D camera={0}; camera.offset=point(offsetX,offsetY); camera.zoom=scale; BeginMode2D(camera);
    DrawRectangle(0,0,1440,960,bg); drawSidebar(data); label("WORKSPACE  /  ANALYTICS",244,27,10,subtle);
    label(page==1?"Transaction explorer":"Business overview",244,54,32,text);
    label(page==1?"Find the story behind every sale.":"A clearer view of your business, in one place.",244,95,14,muted);
    Rectangle export=rect(1240,49,172,40); rounded(export,hover(export)?text:lime); icon(3,export.x+17,export.y+11,bg);
    label("Export region CSV",export.x+43,export.y+13,13,bg); if(hover(export)) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    Rectangle snapshot=rect(1050,49,174,40); rounded(snapshot,hover(snapshot)?raised:surface);
    DrawRectangleRoundedLinesEx(snapshot,0.14f,10,1,edge);
    icon(0,snapshot.x+16,snapshot.y+12,muted); label("Save snapshot",snapshot.x+44,snapshot.y+13,13,text);
    if(hover(snapshot)) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    drawFilterButtons(filterBounds(),selectedRegion);
    char period[96];
    if(data->firstDate[0]) snprintf(period,sizeof period,"%s  -  %s",data->firstDate,data->lastDate);
    else snprintf(period,sizeof period,"No sales in this region");
    icon(4,1180,137,subtle); rightLabel(period,1412,140,12,muted);
    if(page==0) {
        drawKpis(data); drawMonthlyChart(rect(244,340,724,302),data->monthlyRevenue); drawMix(data,rect(988,340,424,302));
        drawTransactions(data); drawRegions(data,*selectedRegion);
    } else drawTransactions(data);
    DrawCircleV(point(249,932),3,lime);
    char status[96]; snprintf(status,sizeof status,"%d records loaded  /  %s",data->loadedCount,regionNames[*selectedRegion]);
    label(status,260,925,11,subtle); rightLabel("1-5  Switch region     /     F10  Save snapshot",1412,925,11,subtle);
    if(!snapshotRendering&&GetTime()<noticeUntil) {
        float w=measure(notice,14)+48; rounded(rect(828-w/2,867,w,43),lime); centered(notice,rect(828-w/2,867,w,43),14,bg);
    }
    EndMode2D();
}

/* Render to a texture to avoid platform DPI differences in screen capture. */
bool saveDashboardSnapshot(const char *filename,const DashboardViewData *data,RegionFilter *selectedRegion)
{
    RenderTexture2D target=LoadRenderTexture(GetScreenWidth(),GetScreenHeight());
    if(target.id==0) return false;
    snapshotRendering=true;
    BeginTextureMode(target);
    drawDashboard(data,selectedRegion);
    EndTextureMode();
    snapshotRendering=false;
    Image image=LoadImageFromTexture(target.texture);
    ImageFlipVertical(&image);
    bool saved=ExportImage(image,filename);
    UnloadImage(image);
    UnloadRenderTexture(target);
    return saved;
}
