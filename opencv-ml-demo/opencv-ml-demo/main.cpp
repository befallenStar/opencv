#include <iostream>
#include <fstream>
#include <vector>
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <ctime>
using namespace std;
using namespace cv;
using namespace ml;

string get_today_time_for_spec()
{
	//获取当前时间
	time_t now = time(0);
	//获取结构化时间
	tm ltm;
	localtime_s(&ltm, &now);
	string time = to_string(ltm.tm_year) + to_string(ltm.tm_mon) + to_string(ltm.tm_mday) +
		to_string(ltm.tm_hour) + to_string(ltm.tm_min) + to_string(ltm.tm_sec);
	return time;
}

Ptr<TrainData> generate_fake_data()
{
	return TrainData::loadFromCSV("result.csv", 1, 0, 1);

}

vector<string> getLabels()
{
	ifstream fp("result.csv");
	vector<string> labels;
	string line;
	getline(fp, line);
	while (getline(fp, line)) {
		stringstream ss(line);
		string str;
		getline(ss, str, ',');
		labels.push_back(str);
	}
	return labels;
}

int main()
{
	string columns[] = {"radarType", "RFMean", "RFMax", "RFMin", "PRIMean", "PRIMax", "PRIMin", "PWMean", "PWMax", "PWMin"};
	string error_columns[] = {"RFMean", "RFMax", "RFMin", "PRIMean", "PRIMax", "PRIMin", "PWMean", "PWMax", "PWMin", "radarType", "predict_type"};

	//获取不同模块路径
	string model_path = "model";
	string info_path = "info";
	string error_path = "error";
	//获取时间
	string time = get_today_time_for_spec();

	bool duplicate_types[] = {true};
	for (bool duplicate_type : duplicate_types) {
		if (duplicate_type)
			string duplicate_flag = "dup";
		else
			string duplicate_flag = "dedup";
		//获取数据
		Ptr<TrainData> data = generate_fake_data();
		//删除缺失的行
		//删除重复行
		//备份数据
		//分割标签
		//划分测试集和训练集
		data->setTrainTestSplitRatio(0.6);
		Mat x_train = data->getTrainSamples();
		//Mat x_train_idx = data->getTrainSampleIdx();
		Mat x_test = data->getTestSamples();
		//Mat x_test_idx = data->getTestSampleIdx();
		//TODO这个方法读到的标签不完整，需要重新从文件中读取标签
		Mat y_train = data->getTrainResponses();
		Mat y_test = data->getTestResponses();
		cout << x_train << endl;
		//cout << x_train_idx << endl;
		cout << x_test << endl;
		//cout << x_test_idx << endl;
		cout << y_train << endl;
		cout << y_test << endl;
		vector<int> train_label;
		for (int i = 0; i < y_train.rows; i++) {
			for (int j = 0; j < y_train.cols; j++) {
				float fLabel = y_train.at<float>(i, j);
				train_label.push_back((int) fLabel);
			}
		}
		Mat train_label_mat = Mat(train_label);
		train_label_mat.reshape(1, 3);
		//SVM训练
		Ptr<SVM> svm = SVM::create();
		svm->setType(SVM::C_SVC); //设置分类器类型
		svm->setKernel(SVM::LINEAR); //设置核函数
		Ptr<TrainData> x_train_data = TrainData::create(x_train, ROW_SAMPLE, train_label_mat);
		svm->train(x_train_data);
		for (int i = 0; i < x_test.rows; i++) {
			//转换成CV_32F类型
			Mat x_test_float;
			x_test_float.create(1, x_test.cols, CV_32F);
			for (int j = 0; j < x_test.cols; j++) {
				x_test_float.at<float>(j) = x_test.at<int>(i, j);
			}
			float response = svm->predict(x_test_float);
			cout << "SVM预测结果：" << response << endl;
			cout << "SVM实际结果：" << y_test.at<float>(i) << endl;
		}
		//决策树
		Ptr<DTrees> dTrees = DTrees::create();
		dTrees->setMaxDepth(8);
		dTrees->setMinSampleCount(2);
		dTrees->setUseSurrogates(false);
		dTrees->setCVFolds(0);
		dTrees->setUse1SERule(false);
		dTrees->setTruncatePrunedTree(false);
		dTrees->train(x_train_data);
		for (int i = 0; i < x_test.rows; i++) {
			//转换成CV_32F类型
			Mat x_test_float;
			x_test_float.create(1, x_test.cols, CV_32F);
			for (int j = 0; j < x_test.cols; j++) {
				x_test_float.at<float>(j) = x_test.at<int>(i, j);
			}
			float response = dTrees->predict(x_test_float);
			cout << "决策树预测结果：" << response << endl;
			cout << "决策树实际结果：" << y_test.at<float>(i) << endl;
		}
		//随机森林
		Ptr<RTrees> rTrees = RTrees::create();
		rTrees->setMaxDepth(8);
		rTrees->setMinSampleCount(2);
		rTrees->setUseSurrogates(false);
		rTrees->setCVFolds(0);
		rTrees->setUse1SERule(false);
		rTrees->setTruncatePrunedTree(false);
		rTrees->train(x_train_data);
		for (int i = 0; i < x_test.rows; i++) {
			//转换成CV_32F类型
			Mat x_test_float;
			x_test_float.create(1, x_test.cols, CV_32F);
			for (int j = 0; j < x_test.cols; j++) {
				x_test_float.at<float>(j) = x_test.at<int>(i, j);
			}
			float response = rTrees->predict(x_test_float);
			cout << "随机森林预测结果：" << response << endl;
			cout << "随机森林实际结果：" << y_test.at<float>(i) << endl;
		}
		//adaboost
		Ptr<Boost> boost = Boost::create();
		boost->setMaxDepth(8);
		boost->setMinSampleCount(2);
		boost->setUseSurrogates(false);
		boost->setCVFolds(0);
		boost->setUse1SERule(false);
		boost->setTruncatePrunedTree(false);
		boost->train(x_train_data);
		for (int i = 0; i < x_test.rows; i++) {
			//转换成CV_32F类型
			Mat x_test_float;
			x_test_float.create(1, x_test.cols, CV_32F);
			for (int j = 0; j < x_test.cols; j++) {
				x_test_float.at<float>(j) = x_test.at<int>(i, j);
			}
			float response = boost->predict(x_test_float);
			cout << "adaboost预测结果：" << response << endl;
			cout << "adaboost实际结果：" << y_test.at<float>(i) << endl;
		}
		//k近邻
		Ptr<KNearest> kNearest = KNearest::create();
		kNearest->train(x_train_data);
		for (int i = 0; i < x_test.rows; i++) {
			//转换成CV_32F类型
			Mat x_test_float;
			x_test_float.create(1, x_test.cols, CV_32F);
			for (int j = 0; j < x_test.cols; j++) {
				x_test_float.at<float>(j) = x_test.at<int>(i, j);
			}
			float response = kNearest->predict(x_test_float);
			cout << "k近邻预测结果：" << response << endl;
			cout << "k近邻实际结果：" << y_test.at<float>(i) << endl;
		}
	}
	return 0;
}