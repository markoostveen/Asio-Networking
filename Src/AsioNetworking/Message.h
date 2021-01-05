#pragma once

#include "config.h"

#include <cstdint>
#include <memory>

#include <type_traits>
#include <cstring>

namespace Networking {
	struct MessageHeader {
		MessageHeader(uint8_t type, int16_t size)
			: Type(type), Category(0), Size(size) {
			Type = type;
			Size = size;
		}

		void SetCategory(uint8_t category) {
			Category = category;
		}

		// header
		uint8_t Type;
		uint8_t Category;
		int16_t Size;
	};

	struct Message {
		Message() = default;
		Message(Message&& message) = default;
		Message(const Message& message) = default;

		template<typename MessageType>
		static Message CreateMessage(const MessageType messageType) {
			Message myMessage;
			myMessage.Header = MessageHeader(static_cast<uint8_t>(messageType), 0);
			myMessage.Body = nullptr;
			return myMessage;
		}

		template<typename DataType>
		DataType& GetData() {
			return *static_cast<DataType*>(static_cast<void*>(Body));
		}

		void Reset() {
			Header.Size = 0;
			Header.Category = 0;
			Header.Type = 0;
			if (Body != nullptr) {
				delete[] Body;
			}
		}

		/// <summary>
		/// Pushing bytes copies given bytes into the message buffer, and increases the packetsize
		/// </summary>
		/// <param name="data"></param>
		/// <param name="size"></param>
		template<typename DataType>
		void Push(DataType& data) {
			Push(&data, sizeof(DataType));
		}

		/// <summary>
		/// Pushing bytes copies given bytes into the message buffer, and increases the packetsize
		/// </summary>
		/// <param name="data"></param>
		/// <param name="size"></param>
		template<typename PointerType>
		void Push(const PointerType data, int size)
		{
			static_assert(std::is_pointer<PointerType>::value);

			uint8_t* newBody = new uint8_t[Header.Size + size];
			std::memcpy(newBody + Header.Size, data, size);
			std::memcpy(newBody, Body, Header.Size);

			Body = newBody;
			Header.Size += size;
		}

		/// <summary>
		/// Pulling bytes from the buffer copies selected bytes into the given pointer and decreases packetsize.
		/// Note bytes are left in the buffer to preven reallocation
		/// </summary>
		/// <param name="data"></param>
		/// <param name="size"></param>
		template<typename DataType>
		void Pull(DataType& data) {
			Pull(&data, sizeof(DataType));
		}

		/// <summary>
		/// Pulling bytes from the buffer copies selected bytes into the given pointer and decreases packetsize.
		/// Note bytes are left in the buffer to preven reallocation
		/// </summary>
		/// <param name="data"></param>
		/// <param name="size"></param>
		template<typename PointerType>
		void Pull(PointerType data, int size)
		{
			static_assert(std::is_pointer<PointerType>::value);

			Header.Size -= size;
			std::memcpy(data, Body + Header.Size, size);
		}

		MessageHeader Header{ 0, 0 };
		uint8_t* Body;
	};
}