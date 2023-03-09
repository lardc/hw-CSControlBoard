﻿#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H

// Команды
#define ACT_ENABLE_POWER				1	// Включение блока
#define ACT_DISABLE_POWER				2	// Выключение блока
#define ACT_CLR_FAULT					3	// Очистка всех fault
#define ACT_CLR_WARNING					4	// Очистка всех warning
//
#define ACT_DBG_DIG_OUT					21	// Проверка работы индикатора на передней панели
#define ACT_DBG_SF_OUT					22	// Проверка работы красного индикатора системы безопасности
#define ACT_DBG_LS_DUT					23	// Проверка работы зеленого индикатора системы безопасности
#define ACT_DBG_LS_ADPTR				24	// Проверка работы коммутации тестовой комбинации
#define ACT_DBG_MEASURE_ID_TOP			25	// Остановка и сборс текущей комбинации
#define ACT_DBG_MEASURE_ID_BOT			26	// Проверка работы сигнала-разрешения системы безопаснсоти
#define ACT_DBG_MEASURE_PRESSURE		27	// Измерение значения напряжение системы самодиагностики

#define ACT_SAVE_TO_ROM					200	// Сохранение пользовательских данных во FLASH процессора
#define ACT_RESTORE_FROM_ROM			201	// Восстановление данных из FLASH
#define ACT_RESET_TO_DEFAULT			202	// Сброс DataTable в состояние по умолчанию

#define ACT_BOOT_LOADER_REQUEST			320	// Перезапуск процессора с целью перепрограммирования
// -----------------------------



// Регистры
// Сохраняемые регистры
#define REG_SFTST_V_ALLOWED_ERR			0	// Допустимая погрешность измеренного значения напряжения (%)


// Несохраняемые регистры чтения-записи
#define REG_DBG							150	// Отладочный регистр
#define REG_RSLT						151	// Отладочный регистр для хранения результатов измерений датчиков

// Регистры только чтение
#define REG_DEV_STATE					192	// Регистр состояния
#define REG_FAULT_REASON				193	// Регистр Fault
#define REG_DISABLE_REASON				194	// Регистр Disable
#define REG_WARNING						195	// Регистр Warning
#define REG_PROBLEM						196	// Регистр Problem
#define REG_OP_RESULT					197	// Регистр результата операции
#define REG_SELF_TEST_OP_RESULT			198	// Регистр результата самотестирования
#define REG_SUB_STATE					199	// Регистр вспомогательного состояния
// -----------------------------
#define REG_FWINFO_SLAVE_NID			256	// Device CAN slave node ID
#define REG_FWINFO_MASTER_NID			257	// Device CAN master node ID (if presented)
// 258 - 259
#define REG_FWINFO_STR_LEN				260	// Length of the information string record
#define REG_FWINFO_STR_BEGIN			261	// Begining of the information string record


// Operation results
#define OPRESULT_NONE					0	// No information or not finished
#define OPRESULT_OK						1	// Operation was successful
#define OPRESULT_FAIL					2	// Operation failed

//  Fault and disable codes
#define DF_NONE							0

// Problem
#define PROBLEM_NONE					0

//  Warning
#define WARNING_NONE					0

//  User Errors
#define ERR_NONE						0
#define ERR_CONFIGURATION_LOCKED		1	//  Устройство защищено от записи
#define ERR_OPERATION_BLOCKED			2	//  Операция не может быть выполнена в текущем состоянии устройства
#define ERR_DEVICE_NOT_READY			3	//  Устройство не готово для смены состояния
#define ERR_WRONG_PWD					4	//  Неправильный ключ

// Endpoints

#endif //  __DEV_OBJ_DIC_H
