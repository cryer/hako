#include <iostream>
#include <iomanip>
#include "inference.h"
#include <filesystem>
#include <fstream>
#include <random>

void Detector(YOLO_V8*& p) {
    std::filesystem::path current_path = std::filesystem::current_path();
    std::filesystem::path imgs_path = current_path / "images";
    for (auto& i : std::filesystem::directory_iterator(imgs_path))
    {
        if (i.path().extension() == ".jpg" || i.path().extension() == ".png" || i.path().extension() == ".jpeg")
        {
            std::string img_path = i.path().string();
            cv::Mat img = cv::imread(img_path);
            std::vector<DL_RESULT> res;
            p->RunSession(img, res);

            for (auto& re : res)
            {
                cv::RNG rng(cv::getTickCount());
                cv::Scalar color(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));

                cv::rectangle(img, re.box, color, 3);

                float confidence = floor(100 * re.confidence) / 100;
                std::cout << std::fixed << std::setprecision(2);
                std::string label = p->classes[re.classId] + " " +
                    std::to_string(confidence).substr(0, std::to_string(confidence).size() - 4);

                cv::rectangle(
                    img,
                    cv::Point(re.box.x, re.box.y - 25),
                    cv::Point(re.box.x + label.length() * 15, re.box.y),
                    color,
                    cv::FILLED
                );

                cv::putText(
                    img,
                    label,
                    cv::Point(re.box.x, re.box.y - 5),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.75,
                    cv::Scalar(0, 0, 0),
                    2
                );


            }
            std::cout << "Press any key to exit" << std::endl;
            cv::imshow("Result of Detection", img);
            cv::waitKey(0);
            //cv::imwrite("E:\\output\\" + std::to_string(k) + ".png", img);
            cv::destroyAllWindows();
        }
    }
}

// 增加视频和摄像头
void Detector(YOLO_V8*& p, const std::string& video_path = "0") {
    // 打开视频源：如果 video_path 是 "0"，则打开默认摄像头；否则打开视频文件
    cv::VideoCapture cap;
    if (video_path == "0") {
        cap.open(0); // 默认摄像头
    } else {
        cap.open(video_path);
    }

    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open video source: " << video_path << std::endl;
        return;
    }

    // 用于计算 FPS
    auto start_time = std::chrono::steady_clock::now();
    int frame_count = 0;
    double fps = 0.0;

    cv::Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cout << "End of video or cannot fetch frame." << std::endl;
            break;
        }

        // 更新帧计数和时间
        frame_count++;
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();

        // 每隔 500ms 更新一次 FPS（避免频繁刷新）
        if (elapsed >= 500) {
            fps = (frame_count * 1000.0) / elapsed;
            frame_count = 0;
            start_time = current_time;
        }

        // 运行 YOLOv8 推理
        std::vector<DL_RESULT> res;
        p->RunSession(frame, res);

        // 随机颜色生成器（可复用，避免每帧重置种子）
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 255);

        // 绘制检测结果
        for (auto& re : res) {
            cv::Scalar color(dis(gen), dis(gen), dis(gen));

            // 绘制边界框
            cv::rectangle(frame, re.box, color, 2);

            // 格式化置信度（保留两位小数）
            float confidence = std::floor(re.confidence * 100.0f + 0.5f) / 100.0f; // 四舍五入到两位小数
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << confidence;
            std::string label = p->classes[re.classId] + " " + ss.str();

            // 绘制标签背景
            int baseline = 0;
            cv::Size label_size = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.75, 2, &baseline);
            cv::rectangle(
                frame,
                cv::Point(re.box.x, re.box.y - label_size.height - 10),
                cv::Point(re.box.x + label_size.width, re.box.y),
                color,
                cv::FILLED
            );

            // 绘制标签文字
            cv::putText(
                frame,
                label,
                cv::Point(re.box.x, re.box.y - 5),
                cv::FONT_HERSHEY_SIMPLEX,
                0.75,
                cv::Scalar(0, 0, 0),
                2
            );
        }

        // 动态设置窗口标题：包含 FPS
        std::ostringstream title;
        title << "YOLOv8 Detection - FPS: " << std::fixed << std::setprecision(1) << fps;
        cv::imshow("YOLOv8 Detection", frame); 
        // 注意：窗口名必须一致才能更新标题
        cv::setWindowTitle("YOLOv8 Detection", title.str());

        // 按 q 或 ESC 退出
        int key = cv::waitKey(1);
        if (key == 'q' || key == 'Q' || key == 27) { // 27 是 ESC 键
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
}

void Classifier(YOLO_V8*& p)
{
    std::filesystem::path current_path = std::filesystem::current_path();
    std::filesystem::path imgs_path = current_path;// / "images"
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 255);
    for (auto& i : std::filesystem::directory_iterator(imgs_path))
    {
        if (i.path().extension() == ".jpg" || i.path().extension() == ".png")
        {
            std::string img_path = i.path().string();
            //std::cout << img_path << std::endl;
            cv::Mat img = cv::imread(img_path);
            std::vector<DL_RESULT> res;
            char* ret = p->RunSession(img, res);

            float positionY = 50;
            for (int i = 0; i < res.size(); i++)
            {
                int r = dis(gen);
                int g = dis(gen);
                int b = dis(gen);
                cv::putText(img, std::to_string(i) + ":", cv::Point(10, positionY), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(b, g, r), 2);
                cv::putText(img, std::to_string(res.at(i).confidence), cv::Point(70, positionY), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(b, g, r), 2);
                positionY += 50;
            }

            cv::imshow("TEST_CLS", img);
            cv::waitKey(0);
            cv::destroyAllWindows();
            //cv::imwrite("E:\\output\\" + std::to_string(k) + ".png", img);
        }

    }
}



int ReadCocoYaml(YOLO_V8*& p) {
    // Open the YAML file
    std::ifstream file("coco.yaml");
    if (!file.is_open())
    {
        std::cerr << "Failed to open file" << std::endl;
        return 1;
    }

    // Read the file line by line
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file, line))
    {
        lines.push_back(line);
    }

    // Find the start and end of the names section
    std::size_t start = 0;
    std::size_t end = 0;
    for (std::size_t i = 0; i < lines.size(); i++)
    {
        if (lines[i].find("names:") != std::string::npos)
        {
            start = i + 1;
        }
        else if (start > 0 && lines[i].find(':') == std::string::npos)
        {
            end = i;
            break;
        }
    }

    // Extract the names
    std::vector<std::string> names;
    for (std::size_t i = start; i < end; i++)
    {
        std::stringstream ss(lines[i]);
        std::string name;
        std::getline(ss, name, ':'); // Extract the number before the delimiter
        std::getline(ss, name); // Extract the string after the delimiter
        names.push_back(name);
    }

    p->classes = names;
    return 0;
}


void DetectTest()
{
    YOLO_V8* yoloDetector = new YOLO_V8;
    ReadCocoYaml(yoloDetector);
    DL_INIT_PARAM params;
    params.rectConfidenceThreshold = 0.1;
    params.iouThreshold = 0.5;
    params.modelPath = "yolov8n.onnx";
    params.imgSize = { 640, 640 };
#ifdef USE_CUDA
    params.cudaEnable = true;

    // GPU FP32 inference
    params.modelType = YOLO_DETECT_V8;
    // GPU FP16 inference
    //Note: change fp16 onnx model
    //params.modelType = YOLO_DETECT_V8_HALF;

#else
    // CPU inference
    params.modelType = YOLO_DETECT_V8;
    params.cudaEnable = false;

#endif
    yoloDetector->CreateSession(params);
    // Detector(yoloDetector);
    Detector(yoloDetector, "../people.mp4");
}


void ClsTest()
{
    YOLO_V8* yoloDetector = new YOLO_V8;
    std::string model_path = "cls.onnx";
    ReadCocoYaml(yoloDetector);
    DL_INIT_PARAM params{ model_path, YOLO_CLS, {224, 224} };
    yoloDetector->CreateSession(params);
    Classifier(yoloDetector);
}


int main()
{
    DetectTest();
    // ClsTest();
}