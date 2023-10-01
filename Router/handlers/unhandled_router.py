from aiogram import Router, F
from aiogram.types import Message

router = Router()

@router.message(F.chat_type != 'private')
async def all_by_private_message(message: Message):
    await message.answer("На данный момент поддерживаются только личные чаты")


@router.message(F.photo)
async def photo_message(message: Message):
    await message.answer("На данный момент фотографии не поддерживаются")

if __name__ == "__main__":
    import os

    os.abort()
