#include <algorithm>
#include <cmath>
#include <vector>

#include "caffe/layers/infogain_loss_layer.hpp"
#include "caffe/util/io.hpp"

namespace caffe {

template <typename Dtype>
void InfogainLossLayer<Dtype>::LayerSetUp(
    const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
  LossLayer<Dtype>::LayerSetUp(bottom, top);
  if (bottom.size() < 3) {
    CHECK(this->layer_param_.infogain_loss_param().has_source())
        << "Infogain matrix source must be specified.";
    BlobProto blob_proto;
    ReadProtoFromBinaryFile(
      this->layer_param_.infogain_loss_param().source(), &blob_proto);
    infogain_.FromProto(blob_proto);
  }
}

template <typename Dtype>
void InfogainLossLayer<Dtype>::Reshape(
    const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
  LossLayer<Dtype>::Reshape(bottom, top);
  Blob<Dtype>* infogain = NULL;
  if (bottom.size() < 3) {
    infogain = &infogain_;
  } else {
    infogain = bottom[2];
  }
  CHECK_EQ(bottom[1]->channels(), 1);
  CHECK_EQ(bottom[1]->height(), 1);
  CHECK_EQ(bottom[1]->width(), 1);
  const int num = bottom[0]->num();
  const int dim = bottom[0]->count() / num;
  CHECK_EQ(infogain->num(), 1);
  CHECK_EQ(infogain->channels(), 1);
  CHECK_EQ(infogain->height(), dim);
  CHECK_EQ(infogain->width(), dim);
}


template <typename Dtype>
void InfogainLossLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
    const vector<Blob<Dtype>*>& top) {
  const Dtype* bottom_data = bottom[0]->cpu_data();
  const Dtype* bottom_label = bottom[1]->cpu_data();
  const Dtype* infogain_mat = NULL;
  if (bottom.size() < 3) {
    infogain_mat = infogain_.cpu_data();
  } else {
    infogain_mat = bottom[2]->cpu_data();
  }
  int num = bottom[0]->num();
  int dim = bottom[0]->count() / bottom[0]->num();
  Dtype loss = 0;
  for (int i = 0; i < num; ++i) {
    int label = static_cast<int>(bottom_label[i]);
    for (int j = 0; j < dim; ++j) {
      Dtype prob = std::max(bottom_data[i * dim + j], Dtype(kLOG_THRESHOLD));
      loss -= infogain_mat[label * dim + j] * log(prob);
    }
  }
  top[0]->mutable_cpu_data()[0] = loss / num;
}

INSTANTIATE_CLASS(InfogainLossLayer);
REGISTER_LAYER_CLASS(InfogainLoss);
}  // namespace caffe
