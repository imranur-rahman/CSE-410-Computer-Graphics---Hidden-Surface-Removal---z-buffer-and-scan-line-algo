#include<bits/stdc++.h>
#include "bitmap_image.hpp"
using namespace std;

int screen_width, screen_height;
double x_left_limit, x_right_limit, y_top_limit, y_bottom_limit, z_start, z_limit;
double dx, dy, top_y, left_x;

struct point{
    double x, y, z;
    point() {}
    point(double a, double b, double c)
    {
        x = a;
        y = b;
        z = c;
    }
};

struct line{
    point a, b;
};

struct triangle{
    point p[3];
    int c[3];
    int id;
    void set_points(point a, point b, point c)
    {
        p[0] = a;
        p[1] = b;
        p[2] = c;
    }
    void set_colors(int r, int g, int b)
    {
        c[0] = r;
        c[1] = g;
        c[2] = b;
    }
    double get_top_point_y()
    {
        return min(p[2].y, y_top_limit);
    }
    double get_bottom_point_y()
    {
        return max(p[0].y, y_bottom_limit);
    }
};

struct edge{
    double x_at_y_min, y_max, y_min, del_x;
    int id_of_polygon;

    bool operator < (const edge& a) const
    {
        return x_at_y_min < a.x_at_y_min;
    }
};

struct planeInfo{
    double a, b, c, d;

    double get_z(double x, double y)
    {
        return (- a * x - b * y - d) / c;
    }
};

struct polygon{
    int id;
    planeInfo plane;
    int color[3];
    bool in_out;
};



struct bucket{
    vector<edge>edges;
};

bool compare_using_y(point const &a, point const &b)
{
    if(a.y != b.y)
        return a.y < b.y;
    return a.x < b.x;
}

ostream& operator<<(ostream& os, const point& p)
{
    return os << p.x << " " << p.y << " " << p.z << endl;
}

ostream& operator<<(ostream& os, const triangle& t)
{
    return os << t.p[0] << t.p[1] << t.p[2] << endl;
}

vector<triangle>triangles;
vector<polygon>polygons;
vector<edge>edges;
vector<bucket>buckets;
int **triangleNumber;

point read_point(ifstream &fin)
{
    point t;
    fin >> t.x >> t.y >> t.z;
    return t;
}

void read_config()
{
    ifstream fin("TestCases/3/config.txt");
    if(fin.is_open() == false)
    {
        cout << "unable to open file";
        exit(1);
    }
    fin >> screen_width >> screen_height;
    fin >> x_left_limit;
    fin >> y_bottom_limit;
    fin >> z_start >> z_limit;
    fin.close();
    x_right_limit = - x_left_limit;
    y_top_limit = - y_bottom_limit;
//    cout << screen_width << screen_height;
//    cout << x_limit;
//    cout << y_limit;
//    cout << z_limit;
}

void read_stage3()
{
    ifstream fin("TestCases/3/stage3.txt");
    if(fin.is_open() == false)
    {
        cout << "unable to open file";
        exit(1);
    }
    point p1, p2, p3;
    int i = 0;
    while(fin >> p1.x >> p1.y >> p1.z)
    {
        cin.eof();
        p2 = read_point(fin);
        p3 = read_point(fin);

        triangle t;
        t.set_points(p1, p2, p3);
        t.set_colors(rand() % 256, rand() % 256, rand() % 256);
        t.id = i;
        sort(t.p, t.p + 3, compare_using_y);
        triangles.push_back(t);
        i++;
    }
    fin.close();
//    cout << triangles.size() << endl;
//    for(int i = 0; i < triangles.size(); ++i)
//        cout << triangles[i] << endl;
}

void read_data()
{
    read_config();
    read_stage3();
}

edge getEdge(triangle t, int i, int j, int polygon_id)
{
    edge ret;
    ret.y_max = max(t.p[i].y, t.p[j].y);
    ret.y_min = min(t.p[i].y, t.p[j].y);

    //cout << ret.y_max << endl;

    if(t.p[i].y < t.p[j].y)
        ret.x_at_y_min = t.p[i].x;
    else
        ret.x_at_y_min = t.p[j].x;

    double m = (t.p[i].y - t.p[j].y) / (t.p[i].x - t.p[j].x);
    ret.del_x = dy / m;

    ret.id_of_polygon = polygon_id;
    return ret;
}

planeInfo generate_plane_info(triangle t, point po)
{
    point p(t.p[1].x - t.p[0].x, t.p[1].y - t.p[0].y, t.p[1].z - t.p[0].z);
    point q(t.p[2].x - t.p[0].x, t.p[2].y - t.p[0].y, t.p[2].z - t.p[0].z);

    planeInfo plane;
    plane.a = p.y * q.z - p.z * q.y;
    plane.b = - p.x * q.z + p.z * q.x;
    plane.c = p.x * q.y - p.y * q.x;
    plane.d = - plane.a * po.x - plane.b * po.y - plane.c * po.z;
    return plane;
}

void create_polygon(triangle t, int id)
{
    polygon p;
    p.id = id;
    p.plane = generate_plane_info(t, t.p[0]);
    p.color[0] = t.c[0];
    p.color[1] = t.c[1];
    p.color[2] = t.c[2];
    p.in_out = false;
    polygons.push_back(p);
}

void create_edge(triangle t, int i)
{
    edge temp;

    //cout << top_y << endl;

    if(t.p[0].y != t.p[1].y){
        temp = getEdge(t, 0, 1, i);
        edges.push_back(temp);
        //int id_in_bucket = (int)round((top_y - temp.y_min) / dy);
        int id_in_bucket = (int)round((temp.y_min - y_bottom_limit) / dy);
        //cout << id_in_bucket << endl;
        if(id_in_bucket >= 0  &&  id_in_bucket < screen_height){
            buckets[id_in_bucket].edges.push_back(temp);
        }
        else
            cout << "some error" << endl;
    }

    if(t.p[1].y != t.p[2].y){
        temp = getEdge(t, 1, 2, i);
        edges.push_back(temp);
        //int id_in_bucket = (int)round((top_y - temp.y_min) / dy);
        int id_in_bucket = (int)round((temp.y_min - y_bottom_limit) / dy);
        //cout << id_in_bucket << endl;
        if(id_in_bucket >= 0  &&  id_in_bucket < screen_height){
            buckets[id_in_bucket].edges.push_back(temp);
        }
        else
            cout << "some error" << endl;
    }

    if(t.p[2].y != t.p[0].y){
        temp = getEdge(t, 2, 0, i);
        edges.push_back(temp);
        //int id_in_bucket = (int)round((top_y - temp.y_min) / dy);
        int id_in_bucket = (int)round((temp.y_min - y_bottom_limit) / dy);
        //cout << id_in_bucket << endl;
        if(id_in_bucket >= 0  &&  id_in_bucket < screen_height){
            buckets[id_in_bucket].edges.push_back(temp);
        }
        else
            cout << "some error" << endl;
    }
}

void initialize_edge_table_and_polygon_table()
{
    dx = (x_right_limit - x_left_limit) / screen_width;
    dy = (y_top_limit - y_bottom_limit) / screen_height;

    top_y = y_top_limit - dy / 2.0;
    left_x = x_left_limit + dx / 2.0;

    triangleNumber = new int*[ screen_height + 7 ];
    for(int i = 0; i < screen_height + 7; ++i)
        triangleNumber[i] = new int[ screen_width + 7 ];

    for(int i = 0; i <= screen_height; ++i)
        for(int j = 0; j <= screen_width; ++j)
            triangleNumber[i][j] = -1;

    buckets.resize(screen_height);
    for(int i = 0; i < triangles.size(); ++i)
    {
        create_polygon(triangles[i], i);

        create_edge(triangles[i], i);

        planeInfo p = polygons[i].plane;
        //cout << p.a << " " << p.b << " " << p.c << " " << p.d << endl;
    }

//    for(int i = 0; i < edges.size(); ++i)
//    {
//        cout << edges[i].del_x << endl;
//    }
}

void apply_procedure()
{
    vector<edge>aet;
    int first_non_empty_bucket = 0;
    while(true){
        if(buckets[first_non_empty_bucket].edges.size() > 0)
            break;
        first_non_empty_bucket++;
    }
    //cout << left_x << endl;
    for(int row = first_non_empty_bucket; row < screen_height; ++row)
    {
        aet.insert(aet.end(), buckets[row].edges.begin(), buckets[row].edges.end());

        sort(aet.begin(), aet.end());

        if(aet.size() == 0)
            continue;

        //smallest x value


        //double y = top_y - row * dy;
        double y = y_bottom_limit + row * dy;

        for(int i = 0; i < aet.size() - 1; ++i)
        {
            int id = aet[i].id_of_polygon;
            polygons[id].in_out = !polygons[id].in_out;

            double x = aet[i].x_at_y_min;//0 chilo age

            double z_min = INT_MAX;
            int min_z_poly_id = -1;

            for(int poly = 0; poly < polygons.size(); ++poly)
            {
                if(polygons[poly].in_out == false)
                    continue;
                double z = polygons[poly].plane.get_z(x, y);
                //cout << x << " " << y << " " << z << endl;
                if(z < z_min)
                {
                    z_min = z;
                    min_z_poly_id = poly;
                }
            }

            //cout << row << " " << z_min << " " << min_z_poly_id << endl;

            if(min_z_poly_id == -1)
                continue;

            double next_x = aet[i + 1].x_at_y_min;

            //cout << row << " " << x << " " << next_x << endl;

            if(x < x_left_limit)
                x = x_left_limit;
            if(next_x > x_right_limit)
                next_x = x_right_limit;

            int left_col = round((x - x_left_limit) / dx);
            int right_col = round((next_x - x_left_limit) / dx);

            for(int col = left_col; col <= right_col; ++col)
            {
                triangleNumber[screen_height - row][col] = min_z_poly_id;
            }
        }

        /*vector<int>toRemove;
        for(int i = 0; i < aet.size(); ++i)
            if(y >= aet[i].y_max)
                toRemove.push_back(i);

        cout << aet.size() << " ";
        for(int i = toRemove.size() - 1; i >= 0; --i)
            aet.erase(aet.begin() + toRemove[i]);
        cout << aet.size() << endl;*/

        //cout << row << " " << aet.size() << " ";
        for(int i = 0; i < aet.size(); ++i)
        {
            int ty = round((aet[i].y_max - y_bottom_limit) / dy);//discrete
            if(row >= ty)
            {
                aet.erase(aet.begin() + i);
                i--;
            }
        }
        //cout << aet.size() << endl;

        for(int i = 0; i < aet.size(); ++i)
            aet[i].x_at_y_min += aet[i].del_x;

        sort(aet.begin(), aet.end());

        for(int i = 0; i < polygons.size(); ++i)
            polygons[i].in_out = false;
    }
}

void save() {


    bitmap_image image(screen_width, screen_height);

    for (int i=0; i<screen_width; i++) {
        for (int j=0; j<screen_height; j++) {
            int id = triangleNumber[i][j];
            if(id == -1)
                image.set_pixel(j, i, 0, 0, 0);
            else
                image.set_pixel(j, i, triangles[id].c[0], triangles[id].c[1], triangles[id].c[2]);
        }
    }

    image.save_image("2.bmp");
    image.clear();
}

void free_memory()
{
    for (int i=0; i<=screen_height; i++){
        delete[] triangleNumber[i];
    }
    delete[] triangleNumber;
    triangles.clear();
    polygons.clear();
    edges.clear();
    buckets.clear();
}

int main()
{
    read_data();
    initialize_edge_table_and_polygon_table();
    apply_procedure();
    save();
    free_memory();
    return 0;
}
