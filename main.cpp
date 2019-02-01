#include <stdio.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <numeric>
#include <chrono>
#include <atomic>
#include <locale>
#include <future>	// C++11: async(); feature<>;
#include <iostream>
#include <fstream>  // std::ofstream

#include <stdlib.h>

#include <opencv2/opencv.hpp>			// C++
#include <opencv2/core/version.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#ifndef CV_VERSION_EPOCH
#include "opencv2/videoio/videoio.hpp"
#define OPENCV_VERSION CVAUX_STR(CV_VERSION_MAJOR)""CVAUX_STR(CV_VERSION_MINOR)""CVAUX_STR(CV_VERSION_REVISION)
#pragma comment(lib, "opencv_world" OPENCV_VERSION ".lib")
#else
#define OPENCV_VERSION CVAUX_STR(CV_VERSION_EPOCH)""CVAUX_STR(CV_VERSION_MAJOR)""CVAUX_STR(CV_VERSION_MINOR)
#pragma comment(lib, "opencv_core" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_imgproc" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_highgui" OPENCV_VERSION ".lib")
#endif


using namespace cv;

std::atomic<bool> right_button_click;
std::atomic<bool> clear_marks;

std::atomic<int> x_start, y_start;
std::atomic<int> x_end, y_end;
std::atomic<int> x_size, y_size;
std::atomic<bool> draw_select, selected;
std::atomic<bool> draw_line;
std::atomic<int> x_line, y_line;
std::atomic<int> left_button_down_count;

std::atomic<int> add_id_img;
Rect prev_img_rect(0, 0, 50, 100);
Rect next_img_rect(1280 - 50, 0, 50, 100);
//my
static bool draw_flag=false;
std::atomic<int> x_show, y_show;
static bool clear_flag = false;
static cv::Point clear_point;
void callback_mouse_click(int event, int x, int y, int flags, void* user_data)
{
    if (event == cv::EVENT_LBUTTONDBLCLK)
    {
        //std::cout << "cv::EVENT_LBUTTONDBLCLK \n";

    }
    else if (event == cv::EVENT_LBUTTONDOWN)
    {

        if (!draw_flag)
        {
            draw_select = true;
            selected = false;
            x_start = x;
            y_start = y;
            draw_line = true;
            x_line = max(x, 0);
            y_line = max(y, 0);
            if (prev_img_rect.contains(Point2i(x, y)))
                add_id_img = -1;
            else if (next_img_rect.contains(Point2i(x, y)))
                add_id_img = 1;
            else
                add_id_img = 0;

            draw_flag = true;
        }
        else
        {
            x_size = abs(x - x_start);
            y_size = abs(y - y_start);
            x_end = max(x, 0);
            y_end = max(y, 0);
            draw_select = false;

            if (x_size > 25 && y_size > 25)
                selected = true;

            draw_flag = false;
        }
        //draw_flag = !draw_flag;


        //std::cout << "cv::EVENT_LBUTTONDOWN \n";
    }
    else if (event == cv::EVENT_LBUTTONUP)
    {
        //x_size = abs(x - x_start);
        //y_size = abs(y - y_start);
        //x_end = max(x, 0);
        //y_end = max(y, 0);
        //draw_select = false;

        //if (x_size>25 && y_size>25)
        //	selected = true;

        //draw_line = false;

        //draw_line = true;
        //std::cout << "cv::EVENT_LBUTTONUP \n";
    }
    else if (event == cv::EVENT_RBUTTONDOWN)
    {
        clear_point.x = x;
        clear_point.y = y;
        clear_flag = true;

        //right_button_click = true;
        //std::cout << "cv::EVENT_RBUTTONDOWN \n";
    }
    if (event == cv::EVENT_RBUTTONDBLCLK)
    {
        std::cout << "cv::EVENT_RBUTTONDBLCLK \n";
    }
    else if (event == cv::EVENT_MOUSEMOVE)
    {

        x_end = max(x, 0);
        y_end = max(y, 0);

        x_show = max(x, 0);
        y_show = max(y, 0);

    }
}

class comma : public std::numpunct<char> {
public:
    comma() : std::numpunct<char>() {}
protected:
    char do_decimal_point() const { return '.'; }
};


int main(int argc, char *argv[])
{
    //sleep(1);
    try
    {
        std::locale loccomma(std::locale::classic(), new comma);
        std::locale::global(loccomma);

        std::string images_path = "./";

        if (argc >= 2) {
            images_path = std::string(argv[1]);         // path to images, train and synset
        }
        else {
            std::cout << "Usage: [path_to_images] [train.txt] [obj.names] \n";
            return 0;
        }

        std::string train_filename = images_path + "train.txt";
        std::string synset_filename = images_path + "obj.names";

        if (argc >= 3) {
            train_filename = std::string(argv[2]);		// file containing: list of images
        }

        if (argc >= 4) {
            synset_filename = std::string(argv[3]);		// file containing: object names
        }

        // capture frames from video file - 1 frame per 3 seconds of video
        if (argc >= 4 && train_filename == "cap_video") {
            const std::string videofile = synset_filename;
            cv::VideoCapture cap(videofile);
            const int fps = cap.get(CV_CAP_PROP_FPS);
            int frame_counter = 0, image_counter = 0;
            float save_each_frames = 50;
            if (argc >= 5) save_each_frames = std::stoul(std::string(argv[4]));

            int pos_filename = 0;
            if ((1 + videofile.find_last_of("\\")) < videofile.length()) pos_filename = 1 + videofile.find_last_of("\\");
            if ((1 + videofile.find_last_of("/")) < videofile.length()) pos_filename = std::max(pos_filename, 1 + (int)videofile.find_last_of("/"));
            std::string const filename = videofile.substr(pos_filename);
            std::string const filename_without_ext = filename.substr(0, filename.find_last_of("."));

            for (cv::Mat frame; cap >> frame, cap.isOpened() && !frame.empty();) {
                cv::imshow("video cap to frames", frame);
#ifndef CV_VERSION_EPOCH
                //        int pressed_key = cv::waitKeyEx(20);	// OpenCV 3.x
                int pressed_key = cv::waitKey(20);	// OpenCV 3.x
#else
                int pressed_key = cv::waitKey(20);		// OpenCV 2.x
#endif
                if (pressed_key == 27 || pressed_key == 1048603) break;  // ESC - exit (OpenCV 2.x / 3.x)
                if (frame_counter++ >= save_each_frames) {		// save frame for each 3 second
                    frame_counter = 0;
                    std::string img_name = images_path + "/" + filename_without_ext + "_" + std::to_string(image_counter++) + ".jpg";
                    std::cout << "saved " << img_name << std::endl;
                    cv::imwrite(img_name, frame);
                }
            }
            exit(0);
        }

        bool show_mouse_coords = true;
        std::vector<std::string> filenames_in_folder;
        //glob(images_path, filenames_in_folder); // void glob(String pattern, std::vector<String>& result, bool recursive = false);
        cv::String images_path_cv = images_path;
        std::vector<cv::String> filenames_in_folder_cv;
        glob(images_path_cv, filenames_in_folder_cv); // void glob(String pattern, std::vector<String>& result, bool recursive = false);
        for (auto &i : filenames_in_folder_cv)
            filenames_in_folder.push_back(i);

        std::vector<std::string> jpg_filenames_path;
        std::vector<std::string> jpg_filenames;
        std::vector<std::string> jpg_filenames_without_ext;
        std::vector<std::string> txt_filenames;
        std::vector<std::string> jpg_in_train;
        std::vector<std::string> synset_txt;
	std::vector<std::string> txt_filenames_w_ext;

        for (auto &i : filenames_in_folder)
        {
            int pos_filename = 0;
            if ((1 + i.find_last_of("\\")) < i.length()) pos_filename = 1 + i.find_last_of("\\");
            if ((1 + i.find_last_of("/")) < i.length()) pos_filename = std::max(pos_filename, 1 + (int)i.find_last_of("/"));


            std::string const filename = i.substr(pos_filename);
            std::string const ext = i.substr(i.find_last_of(".") + 1);
            std::string const filename_without_ext = filename.substr(0, filename.find_last_of("."));

            if (ext == "jpg")
            {
                jpg_filenames_without_ext.push_back(filename_without_ext);
                jpg_filenames.push_back(filename);
                jpg_filenames_path.push_back(i);
            }
            if (ext == "txt") {
                txt_filenames.push_back(filename_without_ext);
		txt_filenames_w_ext.push_back(filename);
            }
        }
        std::sort(jpg_filenames.begin(), jpg_filenames.end());
        std::sort(jpg_filenames_path.begin(), jpg_filenames_path.end());
        std::sort(txt_filenames.begin(), txt_filenames.end());



//        for(int i=0;i<2055;i++){
//          Mat tmp=cv::imread(jpg_filenames_path[i]);
//          std::string tmp_save("/home/ls/easy_label/build-easy_label-Desktop-Debug/dada_else/");
//          tmp_save=tmp_save+jpg_filenames[i];
//          tmp=cv::imread(jpg_filenames_path[i]);
//          cv::imwrite(tmp_save,tmp);

//          std::string tmp_txt(jpg_filenames_path[i]);
//          tmp_txt = tmp_txt.replace(tmp_txt.end()-3, tmp_txt.end(), "txt");  //用str替换从begin位置开始的6个字符
//          tmp_txt=tmp_txt;

//          std::string tmp_txt_save("/home/ls/easy_label/build-easy_label-Desktop-Debug/dada_else/");
//          tmp_txt_save=tmp_txt_save+txt_filenames[i]+".txt";

//          std::string command("cp ");
//          command=command+tmp_txt;
//          command=command+" ";
//          command=command+tmp_txt_save;
//          system(command.c_str());

//        }


        if (jpg_filenames.size() == 0) {
            std::cout << "Error: Image files not found by path: " << images_path << std::endl;
            return 0;
        }


        // intersect jpg & txt
        std::vector<std::string> intersect_filenames(jpg_filenames.size());
        std::vector<std::string> difference_filenames(jpg_filenames.size());

        auto dif_it_end = std::set_difference(jpg_filenames_without_ext.begin(), jpg_filenames_without_ext.end(),
            txt_filenames.begin(), txt_filenames.end(),
            difference_filenames.begin());
        difference_filenames.resize(dif_it_end - difference_filenames.begin());

        auto inter_it_end = std::set_intersection(jpg_filenames_without_ext.begin(), jpg_filenames_without_ext.end(),
            txt_filenames.begin(), txt_filenames.end(),
            intersect_filenames.begin());
        intersect_filenames.resize(inter_it_end - intersect_filenames.begin());

        txt_filenames.clear();
        for (auto &i : intersect_filenames) {
            txt_filenames.push_back(i + ".txt");
        }


        int image_list_count = max(1, (int)jpg_filenames_path.size() - 1);

        // store train.txt
        std::ofstream ofs_train(train_filename, std::ios::out | std::ios::trunc);
        if (!ofs_train.is_open()) {
            throw(std::runtime_error("Can't open file: " + train_filename));
        }

        for (auto &i : intersect_filenames) {
            ofs_train << images_path << "/" << i << ".jpg" << std::endl;
        }
        ofs_train.flush();
        std::cout << "File opened for output: " << train_filename << std::endl;


        // load synset.txt
        {
            std::ifstream ifs(synset_filename);
            if (!ifs.is_open()) {
                throw(std::runtime_error("Can't open file: " + synset_filename));
            }
            for (std::string line; getline(ifs, line);)
                synset_txt.push_back(line);
        }
        std::cout << "File loaded: " << synset_filename << std::endl;

        Mat preview(Size(100, 100), CV_8UC3);
        Mat full_image(Size(1280, 720), CV_8UC3);
        Mat frame(Size(full_image.cols, full_image.rows + preview.rows), CV_8UC3);

        Rect full_rect_dst(Point2i(0, preview.rows), Size(frame.cols, frame.rows - preview.rows));
        Mat full_image_roi = frame(full_rect_dst);

        size_t const preview_number = frame.cols / preview.cols;

        struct coord_t {
            Rect_<float> abs_rect;
            int id;
        };
        std::vector<coord_t> current_coord_vec;
        Size current_img_size;


        std::string const window_name = "Marking images";
        namedWindow(window_name, WINDOW_NORMAL);
        resizeWindow(window_name, 1280, 720);
        imshow(window_name, frame);
        moveWindow(window_name, 0, 0);
        setMouseCallback(window_name, callback_mouse_click);

        bool next_by_click = false;
        bool marks_changed = false;

        int old_trackbar_value = -1, trackbar_value = 0;
        std::string const trackbar_name = "image num";
        int tb_res = createTrackbar(trackbar_name, window_name, &trackbar_value, image_list_count);

        int old_current_obj_id = -1, current_obj_id = 0;
        std::string const trackbar_name_2 = "object id";
        int const max_object_id = (synset_txt.size() > 0) ? synset_txt.size() : 20;
        int tb_res_2 = createTrackbar(trackbar_name_2, window_name, &current_obj_id, max_object_id);

	int delete_this_file = 0;
        do {
            //trackbar_value = min(max(0, trackbar_value), (int)jpg_filenames_path.size() - 1);

            if (old_trackbar_value != trackbar_value)
            {
                draw_flag=false;
                draw_select = false;

                trackbar_value = min(max(0, trackbar_value), (int)jpg_filenames_path.size() - 1);
                setTrackbarPos(trackbar_name, window_name, trackbar_value);
                frame(Rect(0, 0, frame.cols, preview.rows)) = Scalar::all(0);

                // save current coords
                if ((old_trackbar_value >= 0) && (delete_this_file == 0)) // && current_coord_vec.size() > 0) // Yolo v2 can processes background-image without objects
                {
		delete_this_file = 0;
                    try
                    {
                        std::string const jpg_filename = jpg_filenames[old_trackbar_value];
                        std::string const filename_without_ext = jpg_filename.substr(0, jpg_filename.find_last_of("."));
                        std::string const txt_filename = filename_without_ext + ".txt";
                        std::string const txt_filename_path = images_path + "/" + txt_filename;

                        std::cout << "txt_filename_path = " << txt_filename_path << std::endl;

                        std::ofstream ofs(txt_filename_path, std::ios::out | std::ios::trunc);
                        ofs << std::fixed;

                        // store coords to [image name].txt
                        for (auto &i : current_coord_vec)
                        {
                            float const relative_center_x = (float)(i.abs_rect.x + i.abs_rect.width / 2) / full_image_roi.cols;
                            float const relative_center_y = (float)(i.abs_rect.y + i.abs_rect.height / 2) / full_image_roi.rows;
                            float const relative_x = (float)i.abs_rect.x / full_image_roi.cols;
                            float const relative_y = (float)i.abs_rect.y / full_image_roi.rows;
                            float const relative_width = (float)i.abs_rect.width / full_image_roi.cols;
                            float const relative_height = (float)i.abs_rect.height / full_image_roi.rows;

                            if (relative_width <= 0) continue;
                            if (relative_height <= 0) continue;
                            if (relative_center_x <= 0) continue;
                            if (relative_center_y <= 0) continue;

                            ofs << i.id << " " <<
                                relative_center_x << " " << relative_center_y << " " <<
                                relative_width << " " << relative_height << std::endl;
                        }

                        // store [path/image name.jpg] to train.txt
                        auto it = std::find(difference_filenames.begin(), difference_filenames.end(), filename_without_ext);
                        if (it != difference_filenames.end())
                        {
                            ofs_train << images_path << "/" << jpg_filename << std::endl;
                            ofs_train.flush();

                            size_t new_size = std::remove(difference_filenames.begin(), difference_filenames.end(), filename_without_ext) -
                                difference_filenames.begin();
                            difference_filenames.resize(new_size);
                        }
                    }
                    catch (...) { std::cout << " Exception when try to write txt-file \n"; }
                }

                // show preview images
                for (size_t i = 0; i < preview_number && (i + trackbar_value) < jpg_filenames_path.size(); ++i)
                {
                    Mat img = imread(jpg_filenames_path[trackbar_value + i]);
                    // check if the image has been loaded successful to prevent crash
                    if (img.cols == 0)
                    {
                        continue;
                    }
                    resize(img, preview, preview.size());
                    int const x_shift = i*preview.cols + prev_img_rect.width;
                    Rect rect_dst(Point2i(x_shift, 0), preview.size());
                    Mat dst_roi = frame(rect_dst);
                    preview.copyTo(dst_roi);
                    //rectangle(frame, rect_dst, Scalar(200, 150, 200), 2);
                    putText(dst_roi, jpg_filenames[trackbar_value + i], Point2i(0, 10), FONT_HERSHEY_COMPLEX_SMALL, 0.5, Scalar::all(255));

                    if (i == 0)
                    {
                        resize(img, full_image, full_rect_dst.size());
                        full_image.copyTo(full_image_roi);
                        current_img_size = img.size();

                        try {
                            std::string const jpg_filename = jpg_filenames[trackbar_value];
                            std::string const txt_filename = jpg_filename.substr(0, jpg_filename.find_last_of(".")) + ".txt";
                            //std::cout << (images_path + "/" + txt_filename) << std::endl;
                            std::ifstream ifs(images_path + "/" + txt_filename);
                            current_coord_vec.clear();
                            for (std::string line; getline(ifs, line);)
                            {
                                std::stringstream ss(line);
                                coord_t coord;
                                coord.id = -1;
                                ss >> coord.id;
                                if (coord.id < 0) continue;
                                float relative_coord[4] = { -1, -1, -1, -1 };  // rel_center_x, rel_center_y, rel_width, rel_height
                                for (size_t i = 0; i < 4; i++) if (!(ss >> relative_coord[i])) continue;
                                for (size_t i = 0; i < 4; i++) if (relative_coord[i] < 0) continue;
                                coord.abs_rect.x = (relative_coord[0] - relative_coord[2] / 2) * (float)full_image_roi.cols;
                                coord.abs_rect.y = (relative_coord[1] - relative_coord[3] / 2) * (float)full_image_roi.rows;
                                //coord.abs_rect.x = (relative_coord[0]) * (float)full_image_roi.cols;
                                //coord.abs_rect.y = (relative_coord[1]) * (float)full_image_roi.rows;
                                coord.abs_rect.width = relative_coord[2] * (float)full_image_roi.cols;
                                coord.abs_rect.height = relative_coord[3] * (float)full_image_roi.rows;

                                current_coord_vec.push_back(coord);
                            }
                        }
                        catch (...) { std::cout << " Exception when try to read txt-file \n"; }
                    }

                    std::string const jpg_filename = jpg_filenames[trackbar_value + i];
                    std::string const filename_without_ext = jpg_filename.substr(0, jpg_filename.find_last_of("."));
                    if (!std::binary_search(difference_filenames.begin(), difference_filenames.end(), filename_without_ext))
                    {
                        line(dst_roi, Point2i(80, 88), Point2i(85, 93), Scalar(20, 70, 20), 5);
                        line(dst_roi, Point2i(85, 93), Point2i(93, 85), Scalar(20, 70, 20), 5);

                        line(dst_roi, Point2i(80, 88), Point2i(85, 93), Scalar(50, 200, 100), 2);
                        line(dst_roi, Point2i(85, 93), Point2i(93, 85), Scalar(50, 200, 100), 2);
                    }
                }
                std::cout << " trackbar_value = " << trackbar_value << std::endl;

                old_trackbar_value = trackbar_value;

                marks_changed = false;

                rectangle(frame, prev_img_rect, Scalar(100, 100, 100), CV_FILLED);
                rectangle(frame, next_img_rect, Scalar(100, 100, 100), CV_FILLED);
            }

            trackbar_value = min(max(0, trackbar_value), (int)jpg_filenames_path.size() - 1);

            // highlight prev img
            for (size_t i = 0; i < preview_number && (i + trackbar_value) < jpg_filenames_path.size(); ++i)
            {
                int const x_shift = i*preview.cols + prev_img_rect.width;
                Rect rect_dst(Point2i(x_shift, 0), Size(preview.cols - 2, preview.rows));
                Scalar color(100, 70, 100);
                if (i == 0) color = Scalar(250, 120, 150);
                if (y_end < preview.rows && i == (x_end - prev_img_rect.width) / preview.cols) color = Scalar(250, 200, 200);
                rectangle(frame, rect_dst, color, 2);
            }


            if (selected)
            {
                selected = false;
                full_image.copyTo(full_image_roi);

                if (y_end < preview.rows && x_end > prev_img_rect.width && x_end < (full_image.cols - prev_img_rect.width) &&
                    y_start < preview.rows)
                {
                    int const i = (x_end - prev_img_rect.width) / preview.cols;
                    trackbar_value += i;
                }
                else if (y_end >= preview.rows)
                {
                    if (next_by_click) {
                        ++trackbar_value;
                        current_coord_vec.clear();
                    }

                    Rect selected_rect(
                        Point2i((int)min(x_start, x_end), (int)min(y_start, y_end)),
                        Size(x_size, y_size));

                    selected_rect &= full_rect_dst;
                    selected_rect.y -= (int)prev_img_rect.height;

                    coord_t coord;
                    coord.abs_rect = selected_rect;
                    coord.id = current_obj_id;
                    current_coord_vec.push_back(coord);

                    marks_changed = true;
                }
            }

            std::string current_synset_name;
            if (current_obj_id < synset_txt.size()) current_synset_name = "   - " + synset_txt[current_obj_id];

            if (show_mouse_coords) {
                full_image.copyTo(full_image_roi);
                int const x_inside = std::min((int)x_end, full_image_roi.cols);
                int const y_inside = std::min(std::max(0, y_end - (int)prev_img_rect.height), full_image_roi.rows);
                float const relative_center_x = (float)(x_inside) / full_image_roi.cols;
                float const relative_center_y = (float)(y_inside) / full_image_roi.rows;
                int const abs_x = relative_center_x*current_img_size.width;
                int const abs_y = relative_center_y*current_img_size.height;
                char buff[100];
                snprintf(buff, 100, "Abs: %d x %d    Rel: %.3f x %.3f", abs_x, abs_y, relative_center_x, relative_center_y);
                //putText(full_image_roi, buff, Point2i(800, 20), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(50, 10, 10), 3);
                putText(full_image_roi, buff, Point2i(800, 20), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(100, 50, 50), 2);
                putText(full_image_roi, buff, Point2i(800, 20), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(220, 120, 120), 1);
            }
            else
            {
                full_image.copyTo(full_image_roi);
                std::string text = "Show mouse coordinates - press M";
                putText(full_image_roi, text, Point2i(800, 20), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(100, 50, 50), 2);
                //putText(full_image_roi, text, Point2i(800, 20), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(220, 120, 120), 1);
            }


            //if (draw_line)
            {

                Scalar line_arrow_color(200, 150, 100);
                //line(frame, Point(0, y_start), Point(2048, y_start), line_arrow_color, 2);
                //line(frame, Point(x_start, 0), Point(x_start, 2048), line_arrow_color, 2);
                line(frame, Point(0, y_show), Point(2048, y_show), line_arrow_color, 1);
                line(frame, Point(x_show, 100), Point(x_show, 2048), line_arrow_color, 1);


            }

            if (draw_select)
            {
                if (add_id_img != 0) trackbar_value += add_id_img;

                if (y_start >= preview.rows)
                {
                    //full_image.copyTo(full_image_roi);
                    Rect selected_rect(
                        Point2i(max(0, (int)min(x_start, x_end)), max(preview.rows, (int)min(y_start, y_end))),
                        Point2i(max(x_start, x_end), max(y_start, y_end)));
                    rectangle(frame, selected_rect, Scalar(150, 200, 150),2);

                    putText(frame, std::to_string(current_obj_id) + current_synset_name,
                        selected_rect.tl() + Point2i(2, 22), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(150, 200, 150), 2);
                }
            }

            if (clear_flag == true)
            {
                clear_flag = false;
                marks_changed = true;
                full_image.copyTo(full_image_roi);

                std::vector<coord_t>::iterator itt;
                for (itt = current_coord_vec.begin(); itt != current_coord_vec.end();)
                {
                    if ((*itt).abs_rect.tl().x<clear_point.x && (*itt).abs_rect.tl().y<clear_point.y - 100
                        && (*itt).abs_rect.br().x>clear_point.x && (*itt).abs_rect.br().y>clear_point.y - 100)
                    {
                        itt=current_coord_vec.erase(itt);
                    }

                    else
                        itt++;
                }

            }


            if (clear_marks == true)
            {
                clear_marks = false;
                marks_changed = true;
                full_image.copyTo(full_image_roi);

                //\D5\E2\C0\EF\BF\C9\D2Բ\BBȫ\B2\BF\C7\E5\B3\FD
                //current_coord_vec.clear();
                if (current_coord_vec.size())
                    current_coord_vec.pop_back();
            }

            if (right_button_click == true)
            {
                right_button_click = false;
                if (next_by_click)
                {
                    ++trackbar_value;
                }
                else
                {
                    full_image.copyTo(full_image_roi);
                    current_coord_vec.clear();
                }
            }


            if (old_current_obj_id != current_obj_id)
            {
                full_image.copyTo(full_image_roi);
                old_current_obj_id = current_obj_id;
                setTrackbarPos(trackbar_name_2, window_name, current_obj_id);
            }


            for (auto &i : current_coord_vec)
            {
                std::string synset_name;
                if (i.id < synset_txt.size()) synset_name = " - " + synset_txt[i.id];

                int offset = i.id * 25;
                int red = (offset + 0) % 255 * ((i.id + 2) % 3);
                int green = (offset + 70) % 255 * ((i.id + 1) % 3);
                int blue = (offset + 140) % 255 * ((i.id + 0) % 3);
                Scalar color_rect(red, green, blue);    // Scalar color_rect(100, 200, 100);

                putText(full_image_roi, std::to_string(i.id) + synset_name,
                    i.abs_rect.tl() + Point2f(2, 22), FONT_HERSHEY_SIMPLEX, 0.8, color_rect, 2);
                rectangle(full_image_roi, i.abs_rect, color_rect, 2);
            }


            if (next_by_click)
                putText(full_image_roi, "Mode: 1 mark per image (next by click)",
                Point2i(850, 20), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(50, 170, 100), 2);

            {
                std::string const obj_str = "Object id: " + std::to_string(current_obj_id) + current_synset_name;

                putText(full_image_roi, obj_str, Point2i(0, 21), FONT_HERSHEY_DUPLEX, 0.8, Scalar(10, 50, 10), 3);
                putText(full_image_roi, obj_str, Point2i(0, 21), FONT_HERSHEY_DUPLEX, 0.8, Scalar(20, 120, 60), 2);
                putText(full_image_roi, obj_str, Point2i(0, 21), FONT_HERSHEY_DUPLEX, 0.8, Scalar(50, 200, 100), 1);
            }

            putText(full_image_roi,
                "<- prev_img     -> next_img     space - next_img     c - clear_marks     n - one_object_per_img    0-9 - obj_id    ESC - exit",
                Point2i(0, 45), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(50, 10, 10), 2);



            // arrows
            {
                Scalar prev_arrow_color(200, 150, 100);
                Scalar next_arrow_color = prev_arrow_color;
                if (prev_img_rect.contains(Point2i(x_end, y_end))) prev_arrow_color = Scalar(220, 190, 170);
                if (next_img_rect.contains(Point2i(x_end, y_end))) next_arrow_color = Scalar(220, 190, 170);

                std::vector<Point> prev_triangle_pts = { Point(5, 50), Point(40, 90), Point(40, 10), Point(5, 50) };
                Mat prev_roi = frame(prev_img_rect);
                line(prev_roi, prev_triangle_pts[0], prev_triangle_pts[1], prev_arrow_color, 5);
                line(prev_roi, prev_triangle_pts[1], prev_triangle_pts[2], prev_arrow_color, 5);
                line(prev_roi, prev_triangle_pts[2], prev_triangle_pts[3], prev_arrow_color, 5);
                line(prev_roi, prev_triangle_pts[3], prev_triangle_pts[0], prev_arrow_color, 5);

                std::vector<Point> next_triangle_pts = { Point(10, 10), Point(10, 90), Point(45, 50), Point(10, 10) };
                Mat next_roi = frame(next_img_rect);
                line(next_roi, next_triangle_pts[0], next_triangle_pts[1], next_arrow_color, 5);
                line(next_roi, next_triangle_pts[1], next_triangle_pts[2], next_arrow_color, 5);
                line(next_roi, next_triangle_pts[2], next_triangle_pts[3], next_arrow_color, 5);
                line(next_roi, next_triangle_pts[3], next_triangle_pts[0], next_arrow_color, 5);
            }

            imshow(window_name, frame);

#ifndef CV_VERSION_EPOCH
            //int pressed_key = cv::waitKeyEx(20);	// OpenCV 3.x
            int pressed_key = cv::waitKey(20);	// OpenCV 3.x
#else
            int pressed_key = cv::waitKey(20);		// OpenCV 2.x
#endif

            if (pressed_key >= 0)
            for (int i = 0; i < 5; ++i) cv::waitKey(1);

            if (pressed_key == 27 || pressed_key == 1048603)  break;  // ESC - exit

            if (pressed_key >= '0' && pressed_key <= '9') current_obj_id = pressed_key - '0';   // 0 - 9
            if (pressed_key >= 1048624 && pressed_key <= 1048633) current_obj_id = pressed_key - 1048624;   // 0 - 9

            switch (pressed_key)
            {
            case 32:        // SPACE
            case 1048608:	// SPACE
                ++trackbar_value;
                break;

                //      case 2424832:   // <-
                //      case 65361:     // <-
                //      case 91:		// [
            case ',':
                --trackbar_value;
                break;
                //      case 2555904:   // ->
                //      case 65363:     // ->
                //      case 93:		// ]
            case '.':
                ++trackbar_value;
                break;
            case 'c':       // c
            case 1048675:	// c
                clear_marks = true;
                break;
            case 'm':		// m
            case 1048685:   // m
                show_mouse_coords = !show_mouse_coords;
                full_image.copyTo(full_image_roi);
                break;
            case 'n':       // n
            case 1048686:   // n
                next_by_click = !next_by_click;
                full_image.copyTo(full_image_roi);
                break;


            case 'a':
            {
                std::vector<coord_t>::iterator itt;
                for (itt = current_coord_vec.begin(); itt != current_coord_vec.end();)
                {
                    if ((*itt).abs_rect.tl().x<x_show && (*itt).abs_rect.tl().y<y_show - 100
                        && (*itt).abs_rect.br().x>x_show && (*itt).abs_rect.br().y>y_show - 100)
                    {
                        //itt=current_coord_vec.erase(itt);
                        (*itt).id=0;
                        itt++;
                    }

                    else
                        itt++;
                }
                break;
            }
            case 's':
            {
                std::vector<coord_t>::iterator itt;
                for (itt = current_coord_vec.begin(); itt != current_coord_vec.end();)
                {
                    if ((*itt).abs_rect.tl().x<x_show && (*itt).abs_rect.tl().y<y_show - 100
                        && (*itt).abs_rect.br().x>x_show && (*itt).abs_rect.br().y>y_show - 100)
                    {
                        //itt=current_coord_vec.erase(itt);
                        (*itt).id=1;
                        itt++;
                    }

                    else
                        itt++;
                }
                break;
            }
            case 'd':
            {
                std::vector<coord_t>::iterator itt;
                for (itt = current_coord_vec.begin(); itt != current_coord_vec.end();)
                {
                    if ((*itt).abs_rect.tl().x<x_show && (*itt).abs_rect.tl().y<y_show - 100
                        && (*itt).abs_rect.br().x>x_show && (*itt).abs_rect.br().y>y_show - 100)
                    {
                        //itt=current_coord_vec.erase(itt);
                        (*itt).id=2;
                        itt++;
                    }

                    else
                        itt++;
                }
                break;
            }
            case 'l': // delete
            {
		delete_this_file = 1;
            	std::string files_to_be_deleted;
            	files_to_be_deleted = images_path + "/" + jpg_filenames[trackbar_value];
		std::string files_to_be_deleted2 = images_path + "/" + txt_filenames[trackbar_value];
            	printf("deleted: jpg_filename: %s\n", files_to_be_deleted.c_str());
            	std::string cmd = "rm " + files_to_be_deleted;
            	std::string cmd2 = "rm " + files_to_be_deleted2;

		system(cmd2.c_str());
		system(cmd.c_str());
		printf("%s\n", cmd2.c_str());
		
            	break;
            }


            default:
            	//printf("you pressed: %d\n", pressed_key);
                ;
            }

            //if (pressed_key >= 0) std::cout << "pressed_key = " << (int)pressed_key << std::endl;

        } while (true);

    }
    catch (std::exception &e) {
        std::cout << "exception: " << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "unknown exception \n";
    }

    return 0;
}
