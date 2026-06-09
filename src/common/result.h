#pragma once

#include <QString>

template<typename Data> class Result
{
public:
    Result() = default;
    Result(const QString &error, const Data &data) : m_error(error), m_data(data) {}

    static Result<Data> success(const Data &data)
    {
        return Result<Data>("", data);
    }
    static Result<Data> error(const QString &error, const Data &data = {})
    {
        return Result<Data>(error, data);
    }

    bool success() const
    {
        return m_error.isEmpty();
    }
    QString errorMessage() const
    {
        return m_error;
    }
    Data data() const
    {
        return m_data;
    }

    bool operator==(const Result<Data> &other) const
    {
        return m_error == other.m_error && m_data == other.m_data;
    }

private:
    QString m_error;
    Data m_data;
};
