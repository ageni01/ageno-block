import { sidebar } from "vuepress-theme-hope";

export default sidebar({
  "/posts/": ["", { text: "全部文章", children: "structure" }],
});
