// #pragma once
// #include"IQueue.h"

// template <typename T>
// class DynamicQueue : public IQueue<T> {
//     int cap = 1;
//     int start;
//     int end;
//     bool isfull = false;
//     unique_ptr<T[]> arr; 
// public:
//     DynamicQueue():start{0},end{0}{
//         arr = make_unique<T[]>(cap);
//     }
//     DynamicQueue(initializer_list<T> vals):start{0}{
//         cap=vals.size();
//         arr=make_unique<T[]>(cap);
//         for(int i=0;i<cap;i++){
//             arr[i]=*(vals.begin()+i);
//         }
//         end=cap;
//     }

//     void enqueue ( const T & value ) {
//         if(isfull){
//             unique_ptr<T[]> temp = make_unique<T[]>(cap*2);
//             if(start<end){
//                 int k=0;
//                 for(int i=start;i<=end;i++){
//                     temp[k]=arr[i];
//                     k++;
//                 }
//             }
//             else{
//                 int k=0;
//                 for(int i=start;i<cap;i++){
//                     temp[k]=arr[i];
//                     k++;
//                 }
//                 for(int i=0;i<=end;i++){
//                     temp[k]=arr[i];
//                     k++;
//                 }
//             }
//             arr=move(temp);
//             isfull=false;
//             start=0;
//             end=cap;
//             cap*=2;
//         }
//         if(try_enqueue(value)){
//             int preve=end;
//             arr[end++]=value;
//             if(end==cap and start==0){
//                 isfull=true;
//             }
//             else if(end%cap==start){
//                 isfull=true;
//                 end=preve;
//             }
//             else
//                 end=end%cap;
//         }

//     }
//     bool try_enqueue ( const T & value ) {
//         if(empty() or end % cap != start ){
//             return true;
//         }
//         return false;
//     } 
//     T dequeue () {
//         T val;
//         if(try_dequeue(val)){ 
//             val=arr[start++];
//             if(start!=end)
//                 start=start%cap;
//             isfull=false;
//             return val;
//         }
//         isfull=false;
//         throw "Array is empty";
//     }
//     bool try_dequeue ( T & out ) {
//         return !empty();
//     }
//     size_t Size () const{
//         if(start<=end){
//             return end-start;
//         }
//         else{
//             int s=cap-start;
//             s+=end;
//             return s;
//         }
//     }
//     bool empty () const {
//         return start==end; 
//     }
//     size_t capacity () const{
//         return cap;
//     }
//     void display()const{
//         if(start<end){
//             for(int i=start;i<end;i++){
//                 cout<<arr[i]<<" ";
//             }
//         }
//         else if(start>end){
//             for(int i=start;i<cap;i++){
//                 cout<<arr[i]<<" ";
//             }
//             for(int i=0;i<end;i++){
//                 cout<<arr[i]<<" ";
//             }
//         }
//         cout<<endl;
//     }

//     ~DynamicQueue(){}
// };

// #pragma once
// #include"IQ.h"

// template <typename T>
// class DynamicQueue : public IQueue<T> {
//     int cap = 2;
//     int start;
//     int end;
//     bool isfull = false;
//     T* arr;
// public:
//     DynamicQueue() :start{ 0 }, end{ 0 } {
//         arr = new T[cap];
//     }
//     DynamicQueue(initializer_list<T> vals) :start{ 0 } {
//         cap = vals.size();
//         arr = new T[cap];
//         for (int i = 0;i < cap;i++) {
//             arr[i] = *(vals.begin() + i);
//         }
//         end = cap;
//     }

//     void enqueue(const T& value) {
//         if (isfull) {
//             T* temp = new T[cap * 2];
//             if (start < end) {
//                 int k = 0;
//                 for (int i = start;i <= end;i++) {
//                     temp[k] = arr[i];
//                     k++;
//                 }
//             }
//             else {
//                 int k = 0;
//                 for (int i = start;i < cap;i++) {
//                     temp[k] = arr[i];
//                     k++;
//                 }
//                 for (int i = 0;i <= end;i++) {
//                     temp[k] = arr[i];
//                     k++;
//                 }
//             }
//             delete[] arr;
//             arr = temp;
//             temp = nullptr;
//             isfull = false;
//             start = 0;
//             end = cap;
//             cap *= 2;
//         }
//         if (try_enqueue(value)) {
//             int preve = end;
//             arr[end++] = value;
//             if (end == cap and start == 0) {
//                 isfull = true;
//             }
//             else if (end % cap == start) {
//                 isfull = true;
//                 end = preve;
//             }
//             else
//                 end = end % cap;
//         }

//     }
//     bool try_enqueue(const T& value) {
//         if (empty() or end % cap != start) {
//             return true;
//         }
//         return false;
//     }
//     T dequeue() {
//         T val;
//         if (try_dequeue(val)) {
//             val = arr[start++];
//             if (start != end)
//                 start = start % cap;
//             isfull = false;
//             return val;
//         }
//         isfull = false;
//         throw "Array is empty";
//     }
//     bool try_dequeue(T& out) {
//         return !empty();
//     }
//     size_t Size() const {
//         if (start <= end) {
//             return end - start;
//         }
//         else {
//             int s = cap - start;
//             s += end;
//             return s;
//         }
//     }
//     bool empty() const {
//         return start == end;
//     }
//     size_t capacity() const {
//         return cap;
//     }
//     void display()const {
//         if (start < end) {
//             for (int i = start;i < end;i++) {
//                 cout << arr[i] << " ";
//             }
//         }
//         else if (start > end) {
//             for (int i = start;i < cap;i++) {
//                 cout << arr[i] << " ";
//             }
//             for (int i = 0;i < end;i++) {
//                 cout << arr[i] << " ";
//             }
//         }
//         cout << endl;
//     }

//     ~DynamicQueue() {
//         delete[] arr;
//     }
// };



