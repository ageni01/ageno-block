---
cover: /assets/images/cover2.jpg
icon: pen-to-square
date: 2026-01-2
category:
  - mybatis
  - xml的sql
tag:
  - 基础的使用
star: true
sticky: true
---

# mybatis-xml-sql
# 基础的xml骨架
---
```
<?xml version="1.0" encoding="UTF-8" ?>
<!DOCTYPE mapper
  PUBLIC "-//mybatis.org//DTD Mapper 3.0//EN"
  "https://mybatis.org/dtd/mybatis-3-mapper.dtd">
<mapper namespace="org.mybatis.example.BlogMapper">
  <select id="selectBlog" resultType="Blog">
    select * from Blog where id = #{id}
  </select>
</mapper>
```