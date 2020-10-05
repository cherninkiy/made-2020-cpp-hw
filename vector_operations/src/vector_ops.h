#pragma once
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>


namespace task {
    using namespace std;

    namespace collinearity {
        const double EPS = 1e-7;
        /*
         * Вспомогательная функция, выполняет проверку на коллинеарность
         * c точностью eps и выставляет масштабирующий множитель alfa
         */
        template <typename ElemT>
        bool check(const vector<ElemT>& a, const vector<ElemT>& b, double eps, double& alfa) {
            // предикат - проверка на равенство 0
            auto not_zero_predicate = [] (const ElemT elem) { return elem != 0; };

            // поиск первой ненулевой координаты вектора a
            auto a_iter = find_if(a.begin(), a.end(), not_zero_predicate);
            if (a_iter == a.end()) {
                // а - нулевой вектор, он коллинеарен любому вектору
                alfa = 0;
                return true;
            }

            // переход к соответствующей координате вектора b
            auto b_iter = b.begin();
            b_iter += distance(a_iter, a.begin());
            if (any_of(b.begin(), b_iter, not_zero_predicate)) {
                // b - содержит ненулевую координату, а должны быть нули
                alfa = 0;
                return false;
            }

            // масштабирующий множитель (double)
            alfa = *b_iter * 1.0 / *a_iter;
            
            //  проверка соотношения коллинеарности по координатам
            vector<double> prod;
            transform(a_iter, a.end(), b_iter, back_inserter(prod),
                [alfa] (const ElemT a_i, const ElemT b_i) {
                    // либо обе координаты нули, либо их соотношение - alfa
                    return a_i == 0 ? b_i * 1.0 : b_i * 1.0 / a_i - alfa;
                });
            
            // результат проверки - все нули с точностью eps
            return all_of(prod.begin(), prod.end(), 
                [eps] (double x) { return fabs(x) < eps; });
        }
    }

    /*
     * Оператор потокового вывода `<<`, выводящий все значения вектора через пробел, 
     * заканчивая символом переноса строки
     */ 
    template <typename ElemT>
    ostream& operator <<(ostream& os, const vector<ElemT>& vec) {
        for_each(vec.begin(), vec.end(), 
            [&os] (ElemT x) { os << x << " "; });
        os << endl;
        return os;
    }

    /*
     * Оператор потокового ввода `>>`, принимающий первым числом размер, далее значения
     */
    template <typename ElemT>
    istream& operator >>(istream& is, vector<ElemT>& vec) {
        size_t len;
        is >> len;
        vec.resize(len);
        for_each(vec.begin(), vec.end(), [&is] (ElemT& x) { is >> x; });
        return is;
    }

    /*
     * Функция `reverse`, переставляющая элементы вектора в обратном порядке
     */
    template <typename ElemT>
    void reverse(vector<ElemT>& vec) {
        vec = vector<ElemT> { vec.rbegin(), vec.rend() };
    }

    /*
     * Унарный оператор `+`
     */
    template <typename ElemT>
    vector<ElemT> operator +(const vector<ElemT>& vec) {
        return vector<ElemT> { vec.begin(), vec.end() };
    }

    /*
     * Унарный оператор `-`
     */
    template <typename ElemT>
    vector<ElemT> operator -(const vector<ElemT>& vec) {
        vector<ElemT> res(vec.size());
        transform(vec.begin(), vec.end(), res.begin(), 
            [] (ElemT x) -> ElemT { return -x; });
        return res;
    }

    /*
     * Бинарный оператор `+`
     */
    template <typename ElemT>
    vector<ElemT> operator +(const vector<ElemT>& a, const vector<ElemT>& b) {
        vector<ElemT> res;
        transform(a.begin(), a.end(), b.begin(), back_inserter(res),
            [] (ElemT a_i, ElemT b_i) { return a_i + b_i; });
        return res;
    }

    /*
     * Бинарный оператор `-`
     */
    template <typename ElemT>
    vector<ElemT> operator -(const vector<ElemT>& a, const vector<ElemT>& b) {
        vector<ElemT> result;
        transform(a.begin(), a.end(), b.begin(), back_inserter(result),
            [] (ElemT a_i, ElemT b_i) { return a_i - b_i; });
        return result;
    }

    /*
     * Оператор `*`- скалярное произведение
     */
    template <typename ElemT>
    ElemT operator *(const vector<ElemT>& a, const vector<ElemT>& b) {
        vector<ElemT> prod;
        transform(a.begin(), a.end(), b.begin(), back_inserter(prod),
            [] (ElemT a_i, ElemT b_i) { return a_i * b_i; });
        return accumulate(prod.begin(), prod.end(), ElemT(0));
    }   

    /*
     * Оператор `%` - векторное произведение (для векторов размерности 3)
     */
    template <typename ElemT>
    vector<ElemT> operator %(const vector<ElemT>& a, const vector<ElemT>& b) {
        return vector<ElemT> { 
                a[1] * b[2] - a[2] * b[1], 
                a[2] * b[0] - a[0] * b[2], 
                a[0] * b[1] - a[1] * b[0]
            };
    }

    /*
     * Оператор `||` - проверка на коллинеарность
     */
    template <typename ElemT>
    bool operator ||(const vector<ElemT>& a, const vector<ElemT>& b) {
        double alfa = 0;
        return collinearity::check(a, b, collinearity::EPS, alfa);
    }

     /*
     * Оператор `&&` - проверка на сонаправленность
     */
    template <typename ElemT>
    bool operator &&(const vector<ElemT>& a, const vector<ElemT>& b) {
        double alfa = 0;
        return collinearity::check(a, b, collinearity::EPS, alfa) && alfa > 0;
    }   

    /*
     * Оператор `|` - покоординатный логический ИЛИ
     */
    vector<int> operator |(const vector<int>& a, const vector<int>& b) {
        vector<int> res;
        transform(a.begin(), a.end(), b.begin(), back_inserter(res),
            [] (int a_i, int b_i) { return a_i | b_i; });
        return res;
    }

    /*
     * Оператор `&` - покоординатный логический И
     */
    vector<int> operator &(const vector<int>& a, const vector<int>& b) {
        vector<int> res;
        transform(a.begin(), a.end(), b.begin(), back_inserter(res),
            [] (int a_i, int b_i) { return a_i & b_i; });
        return res;
    }
}  // namespace task
