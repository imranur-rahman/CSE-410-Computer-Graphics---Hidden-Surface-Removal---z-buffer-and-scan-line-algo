#include<bits/stdc++.h>
#include "bitmap_image.hpp"
using namespace std;

int screen_width, screen_height;
double x_left_limit, x_right_limit, y_top_limit, y_bottom_limit, z_start, z_limit;
double dx, dy, top_y, left_x;

struct point{
    double x, y, z;
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
double **z_buffer;
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

void initialize_z_buffer_and_frame_buffer()
{
    dx = (x_right_limit - x_left_limit) / screen_width;
    dy = (y_top_limit - y_bottom_limit) / screen_height;

    top_y = y_top_limit - dy / 2.0;
    left_x = x_left_limit + dx / 2.0;

    z_buffer = new double*[ screen_height + 7 ];
    for(int i = 0; i < screen_height + 7; ++i)
        z_buffer[i] = new double[ screen_width + 7 ];

    for(int i = 0; i <= screen_height; ++i)
        for(int j = 0; j <= screen_width; ++j)
            z_buffer[i][j] = z_limit;

    triangleNumber = new int*[ screen_height + 7 ];
    for(int i = 0; i < screen_height + 7; ++i)
        triangleNumber[i] = new int[ screen_width + 7 ];

    for(int i = 0; i <= screen_height; ++i)
        for(int j = 0; j <= screen_width; ++j)
            triangleNumber[i][j] = -1;

}

double get_x(point p, point q, double y)
{
    return p.x + (y - p.y) / (q.y - p.y) * (q.x - p.x);
}
double get_z(point p, point q, double y)
{
    return p.z + (y - p.y) / (q.y - p.y) * (q.z - p.z);
}

void apply_procedure()
{
    for(int i = 0; i < triangles.size(); ++i)
    {
        triangle t = triangles[i];

        double maxY = max(max(t.p[0].y, t.p[1].y), t.p[2].y);
        double minY = min(min(t.p[0].y, t.p[1].y), t.p[2].y);

        double maxX = max(max(t.p[0].x, t.p[1].x), t.p[2].x);
        double minX = min(min(t.p[0].x, t.p[1].x), t.p[2].x);

        double top_scan_line, bottom_scan_line;

        if(maxY < y_top_limit) top_scan_line = maxY;
        else top_scan_line = y_top_limit;

        if(minY > y_bottom_limit) bottom_scan_line = minY;
        else bottom_scan_line = y_bottom_limit;

        int row_start = round((top_y - top_scan_line) / dy);
        int row_end = round((top_y - bottom_scan_line) / dy);

        //if(row_start < 0) row_start = 0;
        //cout << row_start << " " << row_end << endl;
        //cout << y_top_limit << " " << y_bottom_limit << endl;

        for(int row = row_start; row <= row_end; ++row)
        {
            double y = top_y - row * dy;

            vector<pair<int, int> >id;

            if((t.p[0].y <= y  &&  t.p[1].y >= y)  ||  (t.p[1].y <= y  &&  t.p[0].y >= y))
            {
                id.push_back({0, 1});
            }
            if((t.p[2].y <= y  &&  t.p[1].y >= y)  ||  (t.p[1].y <= y  &&  t.p[2].y >= y))
            {
                id.push_back({1, 2});
            }
            if((t.p[0].y <= y  &&  t.p[2].y >= y)  ||  (t.p[2].y <= y  &&  t.p[0].y >= y))
            {
                id.push_back({0, 2});
            }
            else{
                continue;
            }

            //cout << "fadsf" << endl;

            double x1, z1, x2, z2;
            x1 = get_x(t.p[ id[0].first ], t.p[ id[0].second ], y);
            z1 = get_z(t.p[ id[0].first ], t.p[ id[0].second ], y);
            x2 = get_x(t.p[ id[1].first ], t.p[ id[1].second ], y);
            z2 = get_z(t.p[ id[1].first ], t.p[ id[1].second ], y);

            if(x1 > x2)
            {
                swap(x1, x2);
                swap(z1, z2);//ei kaj beshi korchi
            }

            if(x2 > x_right_limit)
                x2 = x_right_limit;
            if(x1 < x_left_limit)
                x1 = x_left_limit;

            int left_col = round((x1 - x_left_limit) / dx);
            int right_col = round((x2 - x_left_limit) / dx);

            if(left_col < 0) left_col = 0;

            //cout << "fadsfadsf" << endl;

            for(int col = left_col; col <= right_col; ++col)
            {
                //cout << row << " " << col << endl;
                double x = left_x + col * dx;
                double z_val = z1 + (x-x1)/(x2-x1) * (z2-z1);

                if(z_val < z_buffer[row][col])
                {
                    z_buffer[row][col] = z_val;
                    triangleNumber[row][col] = i;
                }
            }
        }
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

    image.save_image("1.bmp");

    ofstream outputFile;
    outputFile.open("z_buffer.txt");
    outputFile.precision(7);

    for (int i=0; i<screen_width; i++) {
        bool flag = false;
        for (int j=0; j<screen_height; j++) {
            if (z_buffer[i][j] < z_limit) {
                outputFile<<fixed<<z_buffer[i][j]<<"\t";
                flag = true;
            }
        }
        if (flag) {
            outputFile<<endl;
        }
    }
    outputFile.close();
}

void free_memory() {


    triangles.clear();

    for (int i=0; i<=screen_height; i++){
        delete[] z_buffer[i];
        delete[] triangleNumber[i];
    }
    delete[] z_buffer;
    delete[] triangleNumber;
}


int main()
{
    read_data();
    initialize_z_buffer_and_frame_buffer();
    apply_procedure();
    save();
    free_memory();
    return 0;
}

